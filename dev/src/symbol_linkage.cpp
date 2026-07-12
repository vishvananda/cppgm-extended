#include "symbol_linkage.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "class_template_mangle_info.h"
#include "builtin_type_transforms.h"
#include "cpp_decl_bridge.h"
#include "parser_trace.h"
#include "abi_model.h"
#include "pack_parameter_analysis.h"
#include "semantic_utils.h"
#include "semantic_model.h"

using namespace std;
using namespace cpp_decl;
using namespace template_model;

namespace symbol_linkage {

struct SymbolIdentity::AbiMangleFactEntry
{
  string object_symbol;
  abi_mangle::AbiMangleTarget target;
};

struct SymbolIdentity::AbiMangleFactEntries
{
  vector<AbiMangleFactEntry> entries;
};

SymbolIdentity::SymbolIdentity() = default;

SymbolIdentity::SymbolIdentity(SymbolIdentity && rhs) noexcept = default;

SymbolIdentity & SymbolIdentity::operator=(SymbolIdentity && rhs) noexcept =
    default;

SymbolIdentity::~SymbolIdentity() = default;

static unique_ptr<SymbolIdentity::AbiMangleFactEntries> clone_abi_mangle_facts(
    const unique_ptr<SymbolIdentity::AbiMangleFactEntries> & facts)
{
  unique_ptr<SymbolIdentity::AbiMangleFactEntries> out;
  if(facts) {
    out.reset(new SymbolIdentity::AbiMangleFactEntries(*facts));
  }
  return out;
}

SymbolIdentity::SymbolIdentity(const SymbolIdentity & rhs)
    : internal_symbol(rhs.internal_symbol),
      object_symbol(rhs.object_symbol),
      thread_local_wrapper_object_symbol(rhs.thread_local_wrapper_object_symbol),
      abi_mangle_facts(clone_abi_mangle_facts(rhs.abi_mangle_facts)),
      keep_internal_alias(rhs.keep_internal_alias),
      prefer_local_object_binding(rhs.prefer_local_object_binding),
      linkage(rhs.linkage)
{
}

SymbolIdentity & SymbolIdentity::operator=(const SymbolIdentity & rhs)
{
  if(this == &rhs) {
    return *this;
  }
  internal_symbol = rhs.internal_symbol;
  object_symbol = rhs.object_symbol;
  thread_local_wrapper_object_symbol = rhs.thread_local_wrapper_object_symbol;
  abi_mangle_facts = clone_abi_mangle_facts(rhs.abi_mangle_facts);
  keep_internal_alias = rhs.keep_internal_alias;
  prefer_local_object_binding = rhs.prefer_local_object_binding;
  linkage = rhs.linkage;
  return *this;
}

namespace {

bool & abi_mangle_fact_capture_enabled_ref()
{
  static thread_local bool enabled = false;
  return enabled;
}

bool is_identifier_char(char ch)
{
  return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

bool is_identifier_text_for_mangling(const string & text)
{
  const string trimmed = semantic_utils::trim_space(text);
  if(trimmed.empty() ||
     !(std::isalpha(static_cast<unsigned char>(trimmed[0])) ||
       trimmed[0] == '_')) {
    return false;
  }
  for(size_t i = 1; i < trimmed.size(); ++i) {
    if(!is_identifier_char(trimmed[i])) {
      return false;
    }
  }
  return true;
}

}  // namespace

bool has_object_symbol(const SymbolIdentity & symbol)
{
  return !symbol.object_symbol.empty();
}

bool has_exported_object_symbol(const SymbolIdentity & symbol)
{
  return !symbol.object_symbol.empty() && symbol.linkage != SL_INTERNAL;
}

string exported_object_symbol(const SymbolIdentity & symbol)
{
  return symbol.object_symbol;
}

bool has_weak_linkage(const SymbolIdentity & symbol)
{
  return symbol.linkage == SL_WEAK;
}

AbiMangleFactCaptureScope::AbiMangleFactCaptureScope(bool enabled)
    : previous_enabled(abi_mangle_fact_capture_enabled_ref())
{
  abi_mangle_fact_capture_enabled_ref() = enabled;
}

AbiMangleFactCaptureScope::~AbiMangleFactCaptureScope()
{
  abi_mangle_fact_capture_enabled_ref() = previous_enabled;
}

string mangle_symbol_name(const string & text)
{
  string out;
  for(size_t i = 0; i < text.size(); ++i) {
    if(i + 1 < text.size() && text[i] == ':' && text[i + 1] == ':') {
      out += "__";
      ++i;
      continue;
    }
    const char ch = text[i];
    if((ch >= 'a' && ch <= 'z') ||
       (ch >= 'A' && ch <= 'Z') ||
       (ch >= '0' && ch <= '9') ||
       ch == '_') {
      out += ch;
    } else {
      out += '_';
    }
  }
  return out;
}

string internal_symbol_from_name(const string & name)
{
  return string("@") + mangle_symbol_name(name);
}

string thread_local_wrapper_internal_symbol(const string & variable_internal_symbol)
{
  return variable_internal_symbol + "__tls_wrapper";
}

string thread_local_guard_internal_symbol(const string & variable_internal_symbol)
{
  return variable_internal_symbol + "__tls_guard";
}

struct TemplateParameterMangleContext;
struct TypeMangleContext;

static bool try_emit_type_encoding_ir(const TypePtr & type,
                                      string & out,
                                      const TypeMangleContext * mangle_ctx);
static bool try_emit_special_type_encoding_ir(const TypePtr & type, string & out);
static bool try_emit_qualified_name_encoding_ir(const QualifiedName & qualified,
                                                string & out);
static bool try_emit_qualified_name_object_symbol_ir(const QualifiedName & qualified,
                                                     string & out);
static bool emit_itanium_function_encoding_with_substitutions(
    const QualifiedName & qualified_name,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    string & out,
    vector<abi_mangle::SubstitutionSlot> * substitution_slots = nullptr);

static size_t count_scope_operators(const string & text)
{
  size_t count = 0;
  for(size_t i = 0; i + 1 < text.size(); ++i) {
    if(text[i] == ':' && text[i + 1] == ':') {
      ++count;
      ++i;
    }
  }
  return count;
}

static bool named_type_should_prefer_key_for_mangling(
    const Type & type,
    const string & display_text,
    const string & key_text)
{
  if(key_text.empty()) {
    return false;
  }
  if(type.named_semantic_kind == Type::NSK_TEMPLATE_PARAMETER ||
     type.named_semantic_kind == Type::NSK_DEPENDENT_TYPE ||
     type.named_semantic_kind == Type::NSK_DEPENDENT_ALIAS ||
     type.named_semantic_kind == Type::NSK_DEPENDENT_DECLTYPE ||
     type.named_semantic_kind == Type::NSK_DEPENDENT_TYPEOF ||
     key_text.find("builtin ") == 0) {
    return false;
  }
  if(key_text.find("__local_") != string::npos) {
    return true;
  }
  return count_scope_operators(key_text) > count_scope_operators(display_text);
}

static string special_type_symbol_for_type(abi_mangle::SpecialTypeSymbolKind kind,
                                           const TypePtr & type)
{
  string type_encoding;
  string out;
  if(!try_emit_special_type_encoding_ir(type, type_encoding) ||
     !abi_mangle::emit_special_type_symbol_from_encoding(kind,
                                                         type_encoding,
                                                         out)) {
    return string();
  }
  return out;
}

string typeinfo_symbol_for_type(const TypePtr & type)
{
  return special_type_symbol_for_type(abi_mangle::SPECIAL_TYPEINFO, type);
}

string typeinfo_name_symbol_for_type(const TypePtr & type)
{
  return special_type_symbol_for_type(abi_mangle::SPECIAL_TYPEINFO_NAME, type);
}

string vtable_object_symbol_for_type(const TypePtr & type)
{
  return special_type_symbol_for_type(abi_mangle::SPECIAL_VTABLE, type);
}

string thread_local_wrapper_object_symbol_for_qualified_name(
    const QualifiedName & qualified)
{
  if(qualified.rooted || qualified.name.empty()) {
    return string();
  }
  string encoding;
  if(!try_emit_qualified_name_encoding_ir(qualified, encoding)) {
    return string();
  }
  string wrapper;
  if(!abi_mangle::emit_thread_local_wrapper_symbol_from_encoding(encoding,
                                                                 wrapper)) {
    return string();
  }
  return wrapper;
}

std::string virtual_override_thunk_object_symbol_for_function(
    const QualifiedName & qualified_name,
    const string & display_name,
    bool is_c_linkage,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    long long this_adjust,
    bool has_result_adjust,
    long long result_adjust)
{
  if(is_c_linkage) {
    return string();
  }
  string function_encoding;
  string out;
  if(!emit_itanium_function_encoding_with_substitutions(qualified_name,
                                                       display_name,
                                                       type,
                                                       options,
                                                       function_encoding,
                                                       nullptr) ||
     !abi_mangle::emit_virtual_override_thunk_symbol_from_encoding(
         function_encoding,
         this_adjust,
         has_result_adjust,
         result_adjust,
         out)) {
    return string();
  }
  return out;
}

std::string virtual_override_thunk_object_symbol_for_object_symbol(
    const string & target_object_symbol,
    long long this_adjust,
    bool has_result_adjust,
    long long result_adjust)
{
  string out;
  if(!abi_mangle::emit_virtual_override_thunk_symbol(target_object_symbol,
                                                     this_adjust,
                                                     has_result_adjust,
                                                     result_adjust,
                                                     out)) {
    return string();
  }
  return out;
}

std::string virtual_base_override_thunk_object_symbol_for_function(
    const QualifiedName & qualified_name,
    const string & display_name,
    bool is_c_linkage,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    long long vcall_offset)
{
  if(is_c_linkage) {
    return string();
  }
  string function_encoding;
  string out;
  if(!emit_itanium_function_encoding_with_substitutions(qualified_name,
                                                       display_name,
                                                       type,
                                                       options,
                                                       function_encoding,
                                                       nullptr) ||
     !abi_mangle::emit_virtual_base_override_thunk_symbol_from_encoding(
         function_encoding,
         vcall_offset,
         out)) {
    return string();
  }
  return out;
}

std::string virtual_base_override_thunk_object_symbol_for_object_symbol(
    const string & target_object_symbol,
    long long vcall_offset)
{
  string out;
  if(!abi_mangle::emit_virtual_base_override_thunk_symbol(target_object_symbol,
                                                          vcall_offset,
                                                          out)) {
    return string();
  }
  return out;
}

string construction_vtable_object_symbol(const semantic_model::ClassInfo & dynamic_class,
                                         unsigned long long base_offset,
                                         const semantic_model::ClassInfo & base_class)
{
  string dynamic_encoding;
  string base_encoding;
  if(!try_emit_type_encoding_ir(dynamic_class.type, dynamic_encoding, nullptr) ||
     !try_emit_type_encoding_ir(base_class.type, base_encoding, nullptr)) {
    return string();
  }
  string out;
  if(!abi_mangle::emit_construction_vtable_symbol_from_encodings(
         dynamic_encoding,
         base_offset,
         base_encoding,
         out)) {
    return string();
  }
  return out;
}

string vtt_object_symbol(const semantic_model::ClassInfo & class_info)
{
  return vtt_object_symbol_for_type(class_info.type);
}

string vtt_object_symbol_for_type(const TypePtr & type)
{
  return special_type_symbol_for_type(abi_mangle::SPECIAL_VTT, type);
}

static const SymbolIdentity::AbiMangleFactEntry & abi_mangle_fact_entry_at(
    const SymbolIdentity & symbol,
    size_t index)
{
  if(!symbol.abi_mangle_facts ||
     index >= symbol.abi_mangle_facts->entries.size()) {
    throw logic_error("ABI mangle fact index out of range");
  }
  return symbol.abi_mangle_facts->entries[index];
}

size_t abi_mangle_fact_count(const SymbolIdentity & symbol)
{
  return symbol.abi_mangle_facts ? symbol.abi_mangle_facts->entries.size() : 0;
}

const string & abi_mangle_fact_object_symbol(const SymbolIdentity & symbol,
                                             size_t index)
{
  return abi_mangle_fact_entry_at(symbol, index).object_symbol;
}

unsigned abi_mangle_fact_target_kind(const SymbolIdentity & symbol,
                                     size_t index)
{
  return static_cast<unsigned>(
      abi_mangle_fact_entry_at(symbol, index).target.kind);
}

const string & abi_mangle_fact_target_qualified_name(
    const SymbolIdentity & symbol,
    size_t index)
{
  return abi_mangle_fact_entry_at(symbol, index).target.qualified_name;
}

bool abi_mangle_fact_target_c_linkage(const SymbolIdentity & symbol,
                                      size_t index)
{
  return abi_mangle_fact_entry_at(symbol, index).target.c_linkage;
}

const abi_mangle::AbiMangleTarget & abi_mangle_fact_target(
    const SymbolIdentity & symbol,
    size_t index)
{
  return abi_mangle_fact_entry_at(symbol, index).target;
}

static bool text_uses_only_simple_mangleable_chars(const string & text)
{
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(isalnum(static_cast<unsigned char>(ch)) ||
       isspace(static_cast<unsigned char>(ch)) ||
       ch == '_' ||
       ch == ':' ||
       ch == '<' ||
       ch == '>' ||
       ch == ',' ||
       ch == '.' ||
       ch == '(' ||
       ch == ')' ||
       ch == '[' ||
       ch == ']' ||
       ch == '*' ||
       ch == '&' ||
       ch == '~') {
      continue;
    }
    return false;
  }
  return true;
}

static bool starts_with_literal(const string & text,
                                const char * prefix,
                                size_t prefix_size)
{
  return text.size() >= prefix_size &&
         text.compare(0, prefix_size, prefix) == 0;
}

static size_t stripped_elaborated_type_prefix_offset(const string & text)
{
  if(text.empty()) {
    return 0;
  }
  switch(text[0]) {
  case 'c':
    return starts_with_literal(text, "class ", sizeof("class ") - 1) ?
        sizeof("class ") - 1 :
        0;
  case 'e':
    if(starts_with_literal(text, "enum class ", sizeof("enum class ") - 1)) {
      return sizeof("enum class ") - 1;
    }
    if(starts_with_literal(text, "enum struct ", sizeof("enum struct ") - 1)) {
      return sizeof("enum struct ") - 1;
    }
    return starts_with_literal(text, "enum ", sizeof("enum ") - 1) ?
        sizeof("enum ") - 1 :
        0;
  case 's':
    return starts_with_literal(text, "struct ", sizeof("struct ") - 1) ?
        sizeof("struct ") - 1 :
        0;
  case 'u':
    return starts_with_literal(text, "union ", sizeof("union ") - 1) ?
        sizeof("union ") - 1 :
        0;
  default:
    break;
  }
  return 0;
}

static string strip_elaborated_type_prefix(const string & text)
{
  const size_t offset = stripped_elaborated_type_prefix_offset(text);
  return offset == 0 ? text : text.substr(offset);
}

static bool is_ascii_space(char ch)
{
  return ch == ' ' ||
         ch == '\t' ||
         ch == '\n' ||
         ch == '\r' ||
         ch == '\f' ||
         ch == '\v';
}

static string trim_space(const string & text)
{
  size_t start = 0;
  while(start < text.size() && is_ascii_space(text[start])) {
    ++start;
  }
  size_t end = text.size();
  while(end > start && is_ascii_space(text[end - 1])) {
    --end;
  }
  return text.substr(start, end - start);
}

static string trim_elaborated_type_prefix(const string & text)
{
  size_t start = stripped_elaborated_type_prefix_offset(text);
  while(start < text.size() && is_ascii_space(text[start])) {
    ++start;
  }
  size_t end = text.size();
  while(end > start && is_ascii_space(text[end - 1])) {
    --end;
  }
  return text.substr(start, end - start);
}

static bool text_matches_type_parameter_name(const string & text,
                                             const string & name)
{
  if(name.empty()) {
    return false;
  }
  if(text == name) {
    return true;
  }
  static const char kTypenamePrefix[] = "typename ";
  static const char kClassPrefix[] = "class ";
  const size_t typename_prefix_size = sizeof(kTypenamePrefix) - 1;
  const size_t class_prefix_size = sizeof(kClassPrefix) - 1;
  return (text.size() == typename_prefix_size + name.size() &&
          text.compare(0, typename_prefix_size, kTypenamePrefix) == 0 &&
          text.compare(typename_prefix_size, name.size(), name) == 0) ||
         (text.size() == class_prefix_size + name.size() &&
          text.compare(0, class_prefix_size, kClassPrefix) == 0 &&
          text.compare(class_prefix_size, name.size(), name) == 0);
}

static bool template_parameter_identifier_matches(
    const TemplateParameterInfo & parameter,
    const string & raw_name)
{
  const string name = trim_elaborated_type_prefix(raw_name);
  if(name.empty()) {
    return false;
  }
  if((!parameter.name.empty() &&
      text_matches_type_parameter_name(name, parameter.name)) ||
     (!parameter.placeholder_key.empty() &&
      name == parameter.placeholder_key)) {
    return true;
  }
  for(size_t i = 0; i < parameter.alternate_names.size(); ++i) {
    if(!parameter.alternate_names[i].empty() &&
       name == parameter.alternate_names[i]) {
      return true;
    }
  }
  return false;
}

static string remove_space_chars(const string & text)
{
  string out;
  out.reserve(text.size());
  for(size_t i = 0; i < text.size(); ++i) {
    if(!is_ascii_space(text[i])) {
      out.push_back(text[i]);
    }
  }
  return out;
}

static bool is_signed_decimal_integer_text(const string & text)
{
  const string trimmed = trim_space(text);
  if(trimmed.empty()) {
    return false;
  }
  size_t start = 0;
  if(trimmed[0] == '-' || trimmed[0] == '+') {
    start = 1;
  }
  if(start == trimmed.size()) {
    return false;
  }
  for(size_t i = start; i < trimmed.size(); ++i) {
    if(!isdigit(static_cast<unsigned char>(trimmed[i]))) {
      return false;
    }
  }
  return true;
}

static long long parse_signed_decimal_integer_value(const string & text)
{
  istringstream stream(trim_space(text));
  long long value = 0;
  stream >> value;
  return value;
}

static size_t previous_non_space_for_mangling(const string & text, size_t pos)
{
  while(pos > 0) {
    --pos;
    if(!isspace(static_cast<unsigned char>(text[pos]))) {
      return pos;
    }
  }
  return string::npos;
}

static bool relational_less_operator_at_for_mangling(const string & text,
                                                     size_t pos)
{
  if(pos >= text.size() || text[pos] != '<') {
    return false;
  }
  if(pos + 1 < text.size() && text[pos + 1] == '=') {
    return true;
  }
  const size_t prev = previous_non_space_for_mangling(text, pos);
  return prev != string::npos &&
         (text[prev] == ')' || text[prev] == ']');
}

static bool greater_equal_operator_at_for_mangling(const string & text,
                                                   size_t pos)
{
  return pos + 1 < text.size() && text[pos] == '>' && text[pos + 1] == '=';
}

static bool contains_template_suffix(const string & text)
{
  int depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++depth;
    } else if(ch == '>') {
      if(depth == 0) {
        return true;
      }
      --depth;
    }
  }
  return depth != 0 || text.find('<') != string::npos;
}

struct TemplateComponent
{
  string base_name;
  vector<string> arg_texts;
  bool has_template_id = false;
};

static bool split_template_arguments(const string & text, vector<string> & out)
{
  out.clear();
  string current;
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      if(relational_less_operator_at_for_mangling(text, i)) {
        current.push_back(ch);
        continue;
      }
      ++angle_depth;
      current.push_back(ch);
    } else if(ch == '>') {
      if(greater_equal_operator_at_for_mangling(text, i)) {
        current.push_back(ch);
        continue;
      }
      if(angle_depth == 0) {
        return false;
      }
      --angle_depth;
      current.push_back(ch);
    } else if(ch == '(') {
      ++paren_depth;
      current.push_back(ch);
    } else if(ch == ')') {
      if(paren_depth == 0) {
        return false;
      }
      --paren_depth;
      current.push_back(ch);
    } else if(ch == '[') {
      ++bracket_depth;
      current.push_back(ch);
    } else if(ch == ']') {
      if(bracket_depth == 0) {
        return false;
      }
      --bracket_depth;
      current.push_back(ch);
    } else if(ch == '{') {
      ++brace_depth;
      current.push_back(ch);
    } else if(ch == '}') {
      if(brace_depth == 0) {
        return false;
      }
      --brace_depth;
      current.push_back(ch);
    } else if(ch == ',' && angle_depth == 0 && paren_depth == 0 &&
              bracket_depth == 0 && brace_depth == 0) {
      out.push_back(trim_space(current));
      current.clear();
    } else {
      current.push_back(ch);
    }
  }
  if(angle_depth != 0 || paren_depth != 0 || bracket_depth != 0 || brace_depth != 0) {
    return false;
  }
  if(!current.empty() || !out.empty()) {
    out.push_back(trim_space(current));
  }
  return true;
}

static bool contains_identifier_token(const string & text, const string & name)
{
  if(name.empty()) {
    return false;
  }
  size_t pos = text.find(name);
  while(pos != string::npos) {
    const bool before_ok =
        pos == 0 ||
        !(std::isalnum(static_cast<unsigned char>(text[pos - 1])) ||
          text[pos - 1] == '_');
    const size_t end = pos + name.size();
    const bool after_ok =
        end >= text.size() ||
        !(std::isalnum(static_cast<unsigned char>(text[end])) ||
          text[end] == '_');
    if(before_ok && after_ok) {
      return true;
    }
    pos = text.find(name, pos + 1);
  }
  return false;
}

static string strip_leading_template_disambiguator(const string & text)
{
  const string template_prefix = "template ";
  string out = trim_space(text);
  if(out.compare(0, template_prefix.size(), template_prefix) == 0) {
    out.erase(0, template_prefix.size());
    out = trim_space(out);
  }
  return out;
}

static bool parse_template_component(const string & text, TemplateComponent & out)
{
  out = TemplateComponent();
  const string trimmed = trim_space(text);
  const size_t open = trimmed.find('<');
  const auto normalize_base_name = [](const string & text) -> string
  {
    return strip_leading_template_disambiguator(text);
  };
  if(open == string::npos) {
    out.base_name = normalize_base_name(trimmed);
    return !out.base_name.empty();
  }
  if(trimmed.empty() || trimmed[trimmed.size() - 1] != '>') {
    return false;
  }
  out.has_template_id = true;
  out.base_name = normalize_base_name(trimmed.substr(0, open));
  if(out.base_name.empty()) {
    return false;
  }
  return split_template_arguments(trimmed.substr(open + 1, trimmed.size() - open - 2),
                                  out.arg_texts);
}

struct MangleSubstitutionState
{
  struct SubstitutionKeyHash
  {
    size_t operator()(const abi_mangle::SubstitutionKey & key) const
    {
      if(key.cached_hash != 0) {
        return key.cached_hash;
      }
      size_t seed = static_cast<size_t>(key.kind);
      hash_combine(seed, static_cast<size_t>(key.id));
      hash_combine(seed, std::hash<string>()(key.payload));
      for(size_t i = 0; i < key.children.size(); ++i) {
        hash_combine(seed, operator()(key.children[i]));
      }
      key.cached_hash = seed == 0 ? 1 : seed;
      return key.cached_hash;
    }

    static void hash_combine(size_t & seed, size_t value)
    {
      seed ^= value + static_cast<size_t>(0x9e3779b9) + (seed << 6) + (seed >> 2);
    }
  };

  unordered_map<string, size_t> substitution_index;
  unordered_map<abi_mangle::SubstitutionKey, size_t, SubstitutionKeyHash>
      ir_substitution_index;
  vector<string> substitution_keys;
  vector<abi_mangle::SubstitutionKey> substitution_ir_keys;
  vector<string> substitution_index_log;
  vector<abi_mangle::SubstitutionKey> ir_substitution_index_log;
  MangleSubstitutionState * parent_lookup = nullptr;
  size_t parent_substitution_count = 0;
  size_t checkpoint_depth = 0;
  bool use_entity_substitutions = true;
  bool substitution_capacity_reserved = false;
  bool ir_substitution_index_materialized = false;
};

static const size_t SMALL_IR_SUBSTITUTION_LIMIT = 28;

struct MangleSubstitutionCheckpoint
{
  explicit MangleSubstitutionCheckpoint(MangleSubstitutionState * state)
      : state(state),
        substitution_keys_size(state ? state->substitution_keys.size() : 0),
        substitution_ir_keys_size(
            state ? state->substitution_ir_keys.size() : 0),
        substitution_index_log_size(
            state ? state->substitution_index_log.size() : 0),
        ir_substitution_index_log_size(
            state ? state->ir_substitution_index_log.size() : 0),
        active(state != nullptr)
  {
    if(state) {
      ++state->checkpoint_depth;
    }
  }

  ~MangleSubstitutionCheckpoint()
  {
    rollback();
  }

  void commit()
  {
    if(active && state && state->checkpoint_depth > 0) {
      --state->checkpoint_depth;
      if(state->checkpoint_depth == 0) {
        state->substitution_index_log.clear();
        state->ir_substitution_index_log.clear();
      }
    }
    active = false;
  }

  void rollback()
  {
    if(!active || !state) {
      return;
    }
    for(size_t i = state->ir_substitution_index_log.size();
        i > ir_substitution_index_log_size;
        --i) {
      state->ir_substitution_index.erase(
          state->ir_substitution_index_log[i - 1]);
    }
    state->ir_substitution_index_log.resize(ir_substitution_index_log_size);

    for(size_t i = state->substitution_index_log.size();
        i > substitution_index_log_size;
        --i) {
      state->substitution_index.erase(state->substitution_index_log[i - 1]);
    }
    state->substitution_index_log.resize(substitution_index_log_size);

    state->substitution_keys.resize(substitution_keys_size);
    state->substitution_ir_keys.resize(substitution_ir_keys_size);
    state->ir_substitution_index.clear();
    state->ir_substitution_index_materialized = false;
    if(state->checkpoint_depth > 0) {
      --state->checkpoint_depth;
      if(state->checkpoint_depth == 0) {
        state->substitution_index_log.clear();
        state->ir_substitution_index_log.clear();
      }
    }
    active = false;
  }

  MangleSubstitutionState * state;
  size_t substitution_keys_size;
  size_t substitution_ir_keys_size;
  size_t substitution_index_log_size;
  size_t ir_substitution_index_log_size;
  bool active;

private:
  MangleSubstitutionCheckpoint(const MangleSubstitutionCheckpoint &);
  MangleSubstitutionCheckpoint & operator=(const MangleSubstitutionCheckpoint &);
};

static bool try_mangle_builtin_text(const string & trimmed, string & out);
static bool is_mangleable_builtin_type_transform_name(const string & name);
static bool try_mangle_template_argument_syntax_impl(
    const TemplateArgumentSyntax & syntax,
    const TemplateParameterInfo * parameter,
    string & out,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state);
static bool try_build_type_ir(const TypePtr & type,
                              const TypeMangleContext * mangle_ctx,
                              abi_mangle::Type & out);
static bool try_build_actual_owner_template_reference_type_id_ast_ir(
    const CppAstNode & node,
    const TypePtr & actual_type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out);
static bool non_type_template_parameter_type_is_dependent_for_mangling(
    const TemplateParameterInfo & parameter,
    const TypeMangleContext * mangle_ctx);
static bool type_has_dependent_mangle_state(const TypePtr & type);
static bool template_arguments_have_dependent_mangle_state(
    const vector<TemplateArgument> & arguments);
static bool template_arguments_have_entity_value(
    const vector<TemplateArgument> & arguments);
static string selected_named_type_text(const TypePtr & type);
static void register_function_type_prerequisite_keys(
    const string & substitution_key,
    MangleSubstitutionState * state);
static bool try_mangle_dependent_expression_ast_template_argument(
    const CppAstNode & node,
    string & out,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state);
static bool try_build_type_id_ast_ir(const CppAstNode & node,
                                     const TypeMangleContext * mangle_ctx,
                                     abi_mangle::Type & out);
static bool try_build_type_specifier_seq_ast_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out);
static bool ast_node_value_has_template_or_scope_syntax(const CppAstNode & node);
static bool try_build_dependent_expression_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::DependentExpression & out);
static bool try_build_template_argument_syntax_ir(
    const TemplateArgumentSyntax & syntax,
    const TemplateParameterInfo * parameter,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type::ClassTemplateArgument & out);
static bool try_build_owner_non_type_template_argument_ir(
    const string & text,
    bool pack_expansion,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type::ClassTemplateArgument & out);
static bool try_build_non_type_template_parameter_type_ir(
    const TemplateParameterInfo & parameter,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out);
static bool try_build_cast_target_type_id_ast_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out);
static bool try_build_template_id_type_ir(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out,
    bool expand_alias_templates,
    bool suppress_current_pack_grouping);
static bool alias_parameter_text_matches(const string & text,
                                         const TemplateParameterInfo & parameter);
static bool type_id_ast_is_template_type_parameter(
    const CppAstNode & node,
    const TemplateParameterInfo & parameter);

struct TemplateParameterMangleContext
{
  const vector<TemplateParameterInfo> * parameters = nullptr;
};

struct FunctionParameterMangleInfo
{
  string name;
};

struct TypeMangleContext
{
  const TemplateParameterMangleContext * template_parameters = nullptr;
  const vector<TemplateParameterInfo> * owner_template_parameters = nullptr;
  const vector<TemplateArgument> * owner_template_arguments = nullptr;
  vector<size_t> suppressed_owner_template_argument_indices;
  const vector<FunctionParameterMangleInfo> * function_parameters = nullptr;
  string lexical_scope;
  QualifiedName lexical_scope_syntax;
  const semantic_model::Scope * lookup_scope = nullptr;
  bool allow_direct_std_standard_substitutions = true;
  bool suppress_template_argument_pack_grouping = false;
  bool suppress_template_parameter_type_registration = false;
  const vector<TemplateArgument> * template_arguments = nullptr;
  const map<string, size_t> * template_argument_pack_sizes = nullptr;
  bool prefer_template_argument_values = false;
  bool prefer_concrete_non_type_values_for_dependent_parameter_types = false;
  int alias_expansion_depth = 0;
  bool canonical_enable_if_result_alias_substitutions = false;
  bool suppress_current_type_id_substitution_registration = false;
  bool suppress_expression_qualifier_template_name_substitution = false;
  bool suppress_member_template_name_component_substitution = false;
  bool suppress_decltype_callee_template_prefix_substitution = false;
  bool suppress_type_substitution_keys = false;
  bool prefer_source_template_parameter_expression_arguments = false;
  bool prefer_source_template_name_prefixes_in_expressions = false;
};

struct LambdaContextFunctionSymbolOptionsStorage
{
  explicit LambdaContextFunctionSymbolOptionsStorage(
      const FunctionSymbolOptions & source)
    : options(source)
  {
    // Lambda context metadata can be used after semantic analysis has released
    // its scopes; keep this snapshot self-contained rather than retaining a
    // dangling lookup scope pointer.
    options.lookup_scope = nullptr;
    if(source.template_parameters) {
      template_parameters = *source.template_parameters;
      options.template_parameters = &template_parameters;
    }
    if(source.template_arguments) {
      template_arguments = *source.template_arguments;
      options.template_arguments = &template_arguments;
    }
    if(source.template_argument_pack_sizes) {
      template_argument_pack_sizes = *source.template_argument_pack_sizes;
      options.template_argument_pack_sizes = &template_argument_pack_sizes;
    }
    if(source.owner_template_arguments) {
      if(source.owner_template_arguments == source.template_arguments &&
         options.template_arguments) {
        options.owner_template_arguments = options.template_arguments;
      } else {
        owner_template_arguments = *source.owner_template_arguments;
        options.owner_template_arguments = &owner_template_arguments;
      }
    }
    if(source.owner_template_parameters) {
      if(source.owner_template_parameters == source.template_parameters &&
         options.template_parameters) {
        options.owner_template_parameters = options.template_parameters;
      } else {
        owner_template_parameters = *source.owner_template_parameters;
        options.owner_template_parameters = &owner_template_parameters;
      }
    }
    if(source.owner_mangle_parameters) {
      if(source.owner_mangle_parameters == source.template_parameters &&
         options.template_parameters) {
        options.owner_mangle_parameters = options.template_parameters;
      } else if(source.owner_mangle_parameters == source.owner_template_parameters &&
                options.owner_template_parameters) {
        options.owner_mangle_parameters = options.owner_template_parameters;
      } else {
        owner_mangle_parameters = *source.owner_mangle_parameters;
        options.owner_mangle_parameters = &owner_mangle_parameters;
      }
    }
    if(source.parameter_pattern) {
      parameter_pattern = *source.parameter_pattern;
      options.parameter_pattern = &parameter_pattern;
    }
    if(source.result_type_pattern) {
      result_type_pattern = *source.result_type_pattern;
      options.result_type_pattern = &result_type_pattern;
    }
    if(source.parameter_declarations_pattern) {
      parameter_declaration_nodes.reserve(
          source.parameter_declarations_pattern->size());
      parameter_declarations_pattern.reserve(
          source.parameter_declarations_pattern->size());
      for(size_t i = 0; i < source.parameter_declarations_pattern->size(); ++i) {
        const CppAstNode * parameter = (*source.parameter_declarations_pattern)[i];
        if(parameter) {
          parameter_declaration_nodes.push_back(*parameter);
          parameter_declarations_pattern.push_back(
              &parameter_declaration_nodes.back());
        } else {
          parameter_declarations_pattern.push_back(nullptr);
        }
      }
      options.parameter_declarations_pattern = &parameter_declarations_pattern;
    }
    owner_component_parameters.reserve(source.owner_template_components.size());
    owner_component_arguments.reserve(source.owner_template_components.size());
    owner_component_argument_syntaxes.reserve(
        source.owner_template_components.size());
    for(size_t i = 0; i < source.owner_template_components.size(); ++i) {
      if(source.owner_template_components[i].parameters) {
        if(source.owner_template_components[i].parameters ==
               source.template_parameters &&
           options.template_parameters) {
          options.owner_template_components[i].parameters =
              options.template_parameters;
        } else if(source.owner_template_components[i].parameters ==
                      source.owner_template_parameters &&
                  options.owner_template_parameters) {
          options.owner_template_components[i].parameters =
              options.owner_template_parameters;
        } else {
          owner_component_parameters.push_back(
              *source.owner_template_components[i].parameters);
          options.owner_template_components[i].parameters =
              &owner_component_parameters.back();
        }
      } else {
        options.owner_template_components[i].parameters = nullptr;
      }
      if(source.owner_template_components[i].arguments) {
        if(source.owner_template_components[i].arguments ==
               source.template_arguments &&
           options.template_arguments) {
          options.owner_template_components[i].arguments =
              options.template_arguments;
        } else if(source.owner_template_components[i].arguments ==
                      source.owner_template_arguments &&
                  options.owner_template_arguments) {
          options.owner_template_components[i].arguments =
              options.owner_template_arguments;
        } else {
          owner_component_arguments.push_back(
              *source.owner_template_components[i].arguments);
          options.owner_template_components[i].arguments =
              &owner_component_arguments.back();
        }
      } else {
        options.owner_template_components[i].arguments = nullptr;
      }
      if(source.owner_template_components[i].argument_syntaxes) {
        owner_component_argument_syntaxes.push_back(
            *source.owner_template_components[i].argument_syntaxes);
        options.owner_template_components[i].argument_syntaxes =
            &owner_component_argument_syntaxes.back();
      } else {
        options.owner_template_components[i].argument_syntaxes = nullptr;
      }
    }
  }

  FunctionSymbolOptions options;
  vector<TemplateParameterInfo> template_parameters;
  vector<TemplateArgument> template_arguments;
  map<string, size_t> template_argument_pack_sizes;
  vector<TemplateParameterInfo> owner_template_parameters;
  vector<TemplateParameterInfo> owner_mangle_parameters;
  vector<TemplateArgument> owner_template_arguments;
  vector<pair<string, TypePtr> > parameter_pattern;
  CppAstNode result_type_pattern;
  vector<CppAstNode> parameter_declaration_nodes;
  vector<const CppAstNode *> parameter_declarations_pattern;
  vector<vector<TemplateParameterInfo> > owner_component_parameters;
  vector<vector<TemplateArgument> > owner_component_arguments;
  vector<vector<TemplateArgumentSyntax> > owner_component_argument_syntaxes;
};

shared_ptr<void> make_lambda_context_function_symbol_options(
    const FunctionSymbolOptions & options)
{
  shared_ptr<LambdaContextFunctionSymbolOptionsStorage> storage(
      new LambdaContextFunctionSymbolOptionsStorage(options));
  shared_ptr<FunctionSymbolOptions> options_alias(storage, &storage->options);
  return static_pointer_cast<void>(options_alias);
}

static bool try_emit_special_type_encoding_ir(const TypePtr & type, string & out)
{
  TypeMangleContext mangle_ctx;
  mangle_ctx.prefer_concrete_non_type_values_for_dependent_parameter_types = true;
  return try_emit_type_encoding_ir(type, out, &mangle_ctx);
}

static string template_parameter_scope_payload(
    const vector<TemplateParameterInfo> * parameters)
{
  if(!parameters || parameters->empty()) {
    return string();
  }
  string out = "template-parameters:";
  for(size_t i = 0; i < parameters->size(); ++i) {
    const TemplateParameterInfo & parameter = (*parameters)[i];
    if(i != 0) {
      out += ';';
    }
    out += to_string(static_cast<int>(parameter.kind));
    out += ':';
    out += parameter.parameter_pack ? 'P' : '-';
    out += ':';
    out += parameter.name;
    out += ':';
    out += parameter.placeholder_key;
    out += ':';
    out += to_string(parameter.template_parameter_count);
  }
  return out;
}

static string template_parameter_type_substitution_key(
    const vector<TemplateParameterInfo> * parameters,
    size_t index,
    const TemplateParameterInfo & parameter)
{
  ostringstream key;
  key << "type:tparam(";
  if(parameters) {
    key << "index:" << index
        << ";pack:" << (parameter.parameter_pack ? 1 : 0)
        << ";scope:"
        << template_parameter_scope_payload(parameters);
  } else {
    key << "name:" << parameter.name;
  }
  key << ")";
  return key.str();
}

static bool template_parameter_lists_match(
    const vector<TemplateParameterInfo> * lhs,
    const vector<TemplateParameterInfo> * rhs)
{
  if(!lhs || !rhs || lhs->size() != rhs->size()) {
    return false;
  }
  for(size_t i = 0; i < lhs->size(); ++i) {
    if((*lhs)[i].kind != (*rhs)[i].kind ||
       (*lhs)[i].name != (*rhs)[i].name ||
       (*lhs)[i].parameter_pack != (*rhs)[i].parameter_pack) {
      return false;
    }
  }
  return true;
}

static bool try_emit_type_encoding_ir_impl(const TypePtr & type,
                                           string & out,
                                           const TypeMangleContext * mangle_ctx,
                                           MangleSubstitutionState * state);
static bool try_emit_itanium_function_symbol_ir(
    const QualifiedName & qualified,
    const string & qualified_name_label,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    string & out,
    MangleSubstitutionState * captured_state = nullptr,
    bool complete_plain_parameter_substitutions = false,
    abi_mangle::AbiMangleTarget * captured_target = nullptr);
static bool emit_itanium_function_encoding_with_substitutions(
    const QualifiedName & qualified_name,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    string & out,
    vector<abi_mangle::SubstitutionSlot> * substitution_slots);
static bool build_itanium_function_context_encoding_ir(
    const QualifiedName & qualified,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    abi_mangle::FunctionEncoding & out);

static vector<const Type *> & type_mangle_recursion_stack()
{
  static vector<const Type *> stack;
  return stack;
}

struct TypeMangleRecursionGuard
{
  explicit TypeMangleRecursionGuard(const Type * type)
      : stack(type_mangle_recursion_stack()),
        active(std::find(stack.begin(), stack.end(), type) == stack.end())
  {
    if(active) {
      stack.push_back(type);
    }
  }

  ~TypeMangleRecursionGuard()
  {
    if(active) {
      stack.pop_back();
    }
  }

  bool entered() const { return active; }

private:
  vector<const Type *> & stack;
  bool active;
};
static bool build_type_substitution_key(const TypePtr & type,
                                        const TypeMangleContext * mangle_ctx,
                                        string & out);
static bool build_class_template_specialization_structural_key(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::SubstitutionKey & out);
static bool build_class_template_argument_ir_substitution_keys(
    const vector<abi_mangle::Type::ClassTemplateArgument> & arguments,
    vector<abi_mangle::SubstitutionKey> & out);
static bool build_structural_type_substitution_key(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::SubstitutionKey & out,
    bool allow_fundamental_atom);
static bool build_type_substitution_key_impl(const TypePtr & type,
                                             const TypeMangleContext * mangle_ctx,
                                             string & out,
                                             bool allow_fundamental_atom);

static bool owner_template_argument_index_is_suppressed(
    const TypeMangleContext * mangle_ctx,
    size_t index)
{
  if(!mangle_ctx) {
    return false;
  }
  return find(mangle_ctx->suppressed_owner_template_argument_indices.begin(),
              mangle_ctx->suppressed_owner_template_argument_indices.end(),
              index) !=
         mangle_ctx->suppressed_owner_template_argument_indices.end();
}

static const TypeMangleContext * suppress_owner_template_argument_index(
    const TypeMangleContext * mangle_ctx,
    size_t index,
    TypeMangleContext & storage)
{
  if(!mangle_ctx) {
    return nullptr;
  }
  storage = *mangle_ctx;
  if(!owner_template_argument_index_is_suppressed(mangle_ctx, index)) {
    storage.suppressed_owner_template_argument_indices.push_back(index);
  }
  return &storage;
}

static bool try_mangle_template_argument_impl(const TemplateArgument & arg,
                                              const TemplateParameterInfo * parameter,
                                              string & out,
                                              const TypeMangleContext * mangle_ctx,
                                              MangleSubstitutionState * state,
                                              size_t parameter_index = static_cast<size_t>(-1));

static string function_template_prefix_substitution_key(
    const vector<string> & qualifiers,
    const string & name);
static const CppAstNode * template_parameter_default_payload(
    const TemplateParameterInfo & parameter);

template <typename EmitFn, typename EmitDefaultFn>
static bool emit_grouped_template_arguments(size_t argument_count,
                                            const vector<TemplateParameterInfo> * parameters,
                                            string & out,
                                            EmitFn emit,
                                            EmitDefaultFn emit_default,
                                            const map<string, size_t> * pack_sizes = nullptr)
{
  out += 'I';
  if(!parameters || parameters->empty()) {
    for(size_t i = 0; i < argument_count; ++i) {
      if(!emit(i, static_cast<const TemplateParameterInfo *>(nullptr))) {
        return false;
      }
    }
    out += 'E';
    return true;
  }

  size_t arg_index = 0;
  for(size_t i = 0; i < parameters->size(); ++i) {
    const TemplateParameterInfo & parameter = (*parameters)[i];
    if(parameter.parameter_pack) {
      size_t trailing_nonpack = 0;
      for(size_t j = i + 1; j < parameters->size(); ++j) {
        if(!(*parameters)[j].parameter_pack) {
          ++trailing_nonpack;
        }
      }
      size_t pack_count = argument_count >= arg_index + trailing_nonpack ?
          argument_count - arg_index - trailing_nonpack :
          static_cast<size_t>(-1);
      if(pack_sizes) {
        map<string, size_t>::const_iterator found =
            !parameter.name.empty() ? pack_sizes->find(parameter.name) : pack_sizes->end();
        if(found == pack_sizes->end() && !parameter.placeholder_key.empty()) {
          found = pack_sizes->find(parameter.placeholder_key);
        }
        if(found != pack_sizes->end()) {
          pack_count = found->second;
        }
      }
      if(pack_count == static_cast<size_t>(-1) ||
         argument_count < arg_index + pack_count + trailing_nonpack) {
        return false;
      }
      out += 'J';
      for(size_t j = 0; j < pack_count; ++j) {
        if(!emit(arg_index++, &parameter)) {
          return false;
        }
      }
      out += 'E';
      continue;
    }

    if(arg_index >= argument_count) {
      if(parameter.default_argument) {
        if(!emit_default(parameter)) {
          return false;
        }
        continue;
      }
      return false;
    }
    if(!emit(arg_index++, &parameter)) {
      return false;
    }
  }

  if(arg_index != argument_count) {
    return false;
  }
  out += 'E';
  return true;
}

template <typename EmitFn>
static bool emit_grouped_template_arguments(size_t argument_count,
                                            const vector<TemplateParameterInfo> * parameters,
                                            string & out,
                                            EmitFn emit)
{
  return emit_grouped_template_arguments(
      argument_count,
      parameters,
      out,
      emit,
      [](const TemplateParameterInfo &) -> bool
      {
        return true;
      });
}

static string encode_substitution_sequence_id(size_t index)
{
  if(index == 0) {
    return "S_";
  }

  static const char kDigits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  string digits;
  size_t value = index - 1;
  do {
    digits.push_back(kDigits[value % 36]);
    value /= 36;
  } while(value != 0);

  return string("S") + string(digits.rbegin(), digits.rend()) + "_";
}

static bool insert_substitution_index(MangleSubstitutionState * state,
                                      const string & key,
                                      size_t index);

static bool emit_registered_substitution(MangleSubstitutionState * state,
                                         const string & key,
                                         string & out)
{
  if(!state || !state->use_entity_substitutions || key.empty()) {
    return false;
  }

  if(state->parent_lookup &&
     emit_registered_substitution(state->parent_lookup, key, out)) {
    return true;
  }

  unordered_map<string, size_t>::const_iterator found =
      state->substitution_index.find(key);
  if(found == state->substitution_index.end()) {
    return false;
  }

  out += encode_substitution_sequence_id(found->second);
  return true;
}

static bool insert_ir_substitution_index(
    MangleSubstitutionState * state,
    const abi_mangle::SubstitutionKey & key,
    size_t index);

static void reserve_substitution_state_capacity(MangleSubstitutionState * state)
{
  if(!state || state->substitution_capacity_reserved) {
    return;
  }
  state->substitution_capacity_reserved = true;
  state->substitution_keys.reserve(16);
  state->substitution_ir_keys.reserve(16);
}

static size_t substitution_state_size(const MangleSubstitutionState * state)
{
  return state ? state->parent_substitution_count +
                     state->substitution_keys.size() :
                 0;
}

static bool ir_substitution_key_matches_registered(
    const abi_mangle::SubstitutionKey & registered,
    const abi_mangle::SubstitutionKey & key)
{
  if(registered == key) {
    return true;
  }
  return registered.kind ==
             abi_mangle::SubstitutionKey::SK_CLASS_TEMPLATE_SPECIALIZATION &&
         key.kind == abi_mangle::SubstitutionKey::SK_NAMED &&
         !registered.payload.empty() &&
         registered.payload == key.payload;
}

static bool find_registered_ir_substitution_small(
    const MangleSubstitutionState * state,
    const abi_mangle::SubstitutionKey & key,
    size_t & index)
{
  if(!state ||
     state->substitution_ir_keys.size() > SMALL_IR_SUBSTITUTION_LIMIT) {
    return false;
  }
  for(size_t i = 0; i < state->substitution_ir_keys.size(); ++i) {
    if(ir_substitution_key_matches_registered(state->substitution_ir_keys[i],
                                              key)) {
      index = state->parent_substitution_count + i;
      return true;
    }
  }
  return false;
}

static void materialize_ir_substitution_index(MangleSubstitutionState * state)
{
  if(!state || state->ir_substitution_index_materialized) {
    return;
  }
  state->ir_substitution_index.clear();
  state->ir_substitution_index.reserve(state->substitution_ir_keys.size() * 2);
  for(size_t i = 0; i < state->substitution_ir_keys.size(); ++i) {
    const abi_mangle::SubstitutionKey & key = state->substitution_ir_keys[i];
    if(key.empty()) {
      continue;
    }
    const size_t global_index = state->parent_substitution_count + i;
    state->ir_substitution_index.insert(make_pair(key, global_index));
    if(key.kind ==
           abi_mangle::SubstitutionKey::SK_CLASS_TEMPLATE_SPECIALIZATION &&
       !key.payload.empty()) {
      state->ir_substitution_index.insert(
          make_pair(abi_mangle::SubstitutionKey::named(key.payload),
                    global_index));
    }
  }
  state->ir_substitution_index_materialized = true;
}

static bool find_registered_ir_substitution_index(
    MangleSubstitutionState * state,
    const abi_mangle::SubstitutionKey & key,
    size_t & index)
{
  if(!state || key.empty()) {
    return false;
  }
  if(state->parent_lookup &&
     find_registered_ir_substitution_index(state->parent_lookup, key, index)) {
    return true;
  }
  if(find_registered_ir_substitution_small(state, key, index)) {
    return true;
  }
  if(!state->ir_substitution_index_materialized &&
     state->substitution_ir_keys.size() <= SMALL_IR_SUBSTITUTION_LIMIT) {
    return false;
  }
  materialize_ir_substitution_index(state);

  unordered_map<abi_mangle::SubstitutionKey,
                size_t,
                MangleSubstitutionState::SubstitutionKeyHash>::const_iterator found =
      state->ir_substitution_index.find(key);
  if(found != state->ir_substitution_index.end()) {
    index = found->second;
    return true;
  }
  return false;
}

static bool emit_registered_ir_substitution(
    MangleSubstitutionState * state,
    const abi_mangle::SubstitutionKey & key,
    string & out)
{
  if(!state || !state->use_entity_substitutions || key.empty()) {
    return false;
  }

  size_t index = 0;
  if(find_registered_ir_substitution_index(state, key, index)) {
    out += encode_substitution_sequence_id(index);
    return true;
  }
  return false;
}

static bool insert_substitution_index(MangleSubstitutionState * state,
                                      const string & key,
                                      size_t index)
{
  reserve_substitution_state_capacity(state);
  if(state->substitution_index.empty()) {
    state->substitution_index.reserve(16);
  }
  pair<unordered_map<string, size_t>::iterator, bool> inserted =
      state->substitution_index.insert(make_pair(key, index));
  if(inserted.second && state->checkpoint_depth != 0) {
    state->substitution_index_log.push_back(key);
  }
  return inserted.second;
}

static bool insert_ir_substitution_index(
    MangleSubstitutionState * state,
    const abi_mangle::SubstitutionKey & key,
    size_t index)
{
  reserve_substitution_state_capacity(state);
  if(!state->ir_substitution_index_materialized &&
     state->substitution_ir_keys.size() < SMALL_IR_SUBSTITUTION_LIMIT) {
    return true;
  }
  materialize_ir_substitution_index(state);
  pair<unordered_map<abi_mangle::SubstitutionKey,
                     size_t,
                     MangleSubstitutionState::SubstitutionKeyHash>::iterator,
       bool> inserted =
      state->ir_substitution_index.insert(make_pair(key, index));
  if(inserted.second && state->checkpoint_depth != 0) {
    state->ir_substitution_index_log.push_back(key);
  }
  if(inserted.second &&
     key.kind ==
         abi_mangle::SubstitutionKey::SK_CLASS_TEMPLATE_SPECIALIZATION &&
     !key.payload.empty()) {
    const abi_mangle::SubstitutionKey alias =
        abi_mangle::SubstitutionKey::named(key.payload);
    pair<unordered_map<abi_mangle::SubstitutionKey,
                       size_t,
                       MangleSubstitutionState::SubstitutionKeyHash>::iterator,
         bool> alias_inserted =
        state->ir_substitution_index.insert(make_pair(alias, index));
    if(alias_inserted.second && state->checkpoint_depth != 0) {
      state->ir_substitution_index_log.push_back(alias);
    }
  }
  return inserted.second;
}

static void ensure_substitution_ir_key_slots(MangleSubstitutionState * state)
{
  if(!state) {
    return;
  }
  while(state->substitution_ir_keys.size() < state->substitution_keys.size()) {
    state->substitution_ir_keys.push_back(
        abi_mangle::SubstitutionKey::none());
  }
}

static vector<abi_mangle::SubstitutionSlot>
substitution_slots_from_state(MangleSubstitutionState * state)
{
  vector<abi_mangle::SubstitutionSlot> slots;
  if(!state) {
    return slots;
  }
  ensure_substitution_ir_key_slots(state);
  slots.reserve(state->substitution_keys.size());
  for(size_t i = 0; i < state->substitution_keys.size(); ++i) {
    const abi_mangle::SubstitutionKey ir_key =
        i < state->substitution_ir_keys.size() ?
            state->substitution_ir_keys[i] :
            abi_mangle::SubstitutionKey::none();
    if(!ir_key.empty()) {
      slots.push_back(abi_mangle::SubstitutionSlot::typed(ir_key));
    }
  }
  return slots;
}

static void register_substitution_key_raw(MangleSubstitutionState * state,
                                          const string & key)
{
  if(!state || key.empty()) {
    return;
  }
  if(state->parent_lookup) {
    string ignored;
    if(emit_registered_substitution(state->parent_lookup, key, ignored)) {
      return;
    }
  }
  const size_t index = substitution_state_size(state);
  if(insert_substitution_index(state, key, index)) {
    state->substitution_keys.push_back(key);
    state->substitution_ir_keys.push_back(
        abi_mangle::SubstitutionKey::none());
  }
}

static bool prepare_ir_substitution_key_registration(
    MangleSubstitutionState * state,
    const abi_mangle::SubstitutionKey & key)
{
  if(!state || key.empty()) {
    return false;
  }
  size_t existing_index = 0;
  if(state->parent_lookup &&
     find_registered_ir_substitution_index(state->parent_lookup,
                                           key,
                                           existing_index)) {
    return false;
  }
  if(find_registered_ir_substitution_small(state, key, existing_index)) {
    return false;
  }
  const size_t index = substitution_state_size(state);
  return insert_ir_substitution_index(state, key, index);
}

static void register_ir_substitution_key_raw(
    MangleSubstitutionState * state,
    const abi_mangle::SubstitutionKey & key)
{
  if(prepare_ir_substitution_key_registration(state, key)) {
    state->substitution_keys.push_back(string());
    state->substitution_ir_keys.push_back(key);
  }
}

static void register_ir_substitution_key_owned(
    MangleSubstitutionState * state,
    abi_mangle::SubstitutionKey && key)
{
  if(prepare_ir_substitution_key_registration(state, key)) {
    state->substitution_keys.push_back(string());
    state->substitution_ir_keys.push_back(std::move(key));
  }
}

static void register_ir_substitution_key(
    MangleSubstitutionState * state,
    const abi_mangle::SubstitutionKey & key)
{
  register_ir_substitution_key_raw(state, key);
}

static void register_substitution_key(MangleSubstitutionState * state,
                                      const string & key)
{
  if(!state || key.empty() || state->substitution_index.count(key) != 0) {
    return;
  }
  if(state->parent_lookup) {
    string ignored;
    if(emit_registered_substitution(state->parent_lookup, key, ignored)) {
      return;
    }
  }
  register_function_type_prerequisite_keys(key, state);
  register_substitution_key_raw(state, key);
}

static void register_substitution_key_if_absent(MangleSubstitutionState * state,
                                                const string & key)
{
  if(!state || key.empty() ||
     state->substitution_index.find(key) != state->substitution_index.end()) {
    return;
  }
  register_substitution_key(state, key);
}

static bool is_template_parameter_substitution_key(const string & key)
{
  return key.find("type:tparam(") == 0 ||
         key.find("template-arg-pack(type:tparam(") == 0;
}

static const abi_mangle::SubstitutionKey & substitution_ir_key_at(
    const MangleSubstitutionState & state,
    size_t index)
{
  static const abi_mangle::SubstitutionKey empty_key;
  return index < state.substitution_ir_keys.size() ?
      state.substitution_ir_keys[index] :
      empty_key;
}

static bool substitution_ir_key_is_template_parameter(
    const abi_mangle::SubstitutionKey & key)
{
  if(key.kind == abi_mangle::SubstitutionKey::SK_TYPE_TEMPLATE_PARAMETER) {
    return true;
  }
  if(key.kind == abi_mangle::SubstitutionKey::SK_TEMPLATE_ARGUMENT_TYPE &&
     key.children.size() == 1) {
    return substitution_ir_key_is_template_parameter(key.children[0]);
  }
  if(key.kind == abi_mangle::SubstitutionKey::SK_TYPE &&
     key.payload.find("template-arg-pack:") == 0) {
    return true;
  }
  return false;
}

static bool substitution_slot_is_template_parameter(
    const string & key,
    const abi_mangle::SubstitutionKey & ir_key)
{
  return is_template_parameter_substitution_key(key) ||
         substitution_ir_key_is_template_parameter(ir_key);
}

static bool substitution_slot_is_template_prefix(
    const string & key,
    const abi_mangle::SubstitutionKey & ir_key)
{
  return key.find("template-prefix:") == 0 ||
         ir_key.kind == abi_mangle::SubstitutionKey::SK_PREFIX;
}

static bool substitution_slot_is_name(
    const string & key,
    const abi_mangle::SubstitutionKey & ir_key)
{
  return key.find("name:") == 0 ||
         ir_key.kind == abi_mangle::SubstitutionKey::SK_NAMED;
}

static bool substitution_slot_is_decay_alias_name(
    const string & key,
    const abi_mangle::SubstitutionKey & ir_key)
{
  return key.find("name:__decay_t<") == 0 ||
         (ir_key.kind == abi_mangle::SubstitutionKey::SK_NAMED &&
          ir_key.payload.find("__decay_t<") == 0);
}

static bool substitution_slot_is_member_named_type(
    const string & key,
    const abi_mangle::SubstitutionKey & ir_key)
{
  return key.find("type:member-named:") == 0 ||
         (ir_key.kind == abi_mangle::SubstitutionKey::SK_TYPE &&
          ir_key.payload.find("member-named:") == 0);
}

static void register_substitution_slot_if_absent(
    MangleSubstitutionState * state,
    const string & key,
    const abi_mangle::SubstitutionKey & ir_key)
{
  if(!state) {
    return;
  }
  if(!ir_key.empty()) {
    register_ir_substitution_key(state, ir_key);
    return;
  }
  register_substitution_key_if_absent(state, key);
}

static MangleSubstitutionState fork_dependent_parameter_substitution_state(
    MangleSubstitutionState * parent)
{
  MangleSubstitutionState child;
  if(parent) {
    child.parent_lookup = parent;
    child.parent_substitution_count = substitution_state_size(parent);
    child.use_entity_substitutions = parent->use_entity_substitutions;
  }
  return child;
}

static void merge_dependent_parameter_type_substitutions(
    const MangleSubstitutionState & source,
    MangleSubstitutionState * target)
{
  if(!target) {
    return;
  }
  const bool source_is_fork =
      source.parent_lookup == target &&
      source.parent_substitution_count == substitution_state_size(target);
  const size_t first_new_index = source_is_fork ?
      0 :
      target->substitution_keys.size();
  size_t first_template_parameter_index = source.substitution_keys.size();
  bool has_template_prefix_before_parameter = false;
  bool has_decay_alias_after_parameter = false;
  if(parser_trace::enabled("symbol.linkage")) {
    ostringstream trace;
    trace << "dependent-parameter-merge begin target-size="
          << first_new_index
          << " source-size=" << source.substitution_keys.size();
    parser_trace::note("symbol.linkage", string(), trace.str());
  }
  for(size_t i = first_new_index; i < source.substitution_keys.size(); ++i) {
    const string & key = source.substitution_keys[i];
    const abi_mangle::SubstitutionKey & ir_key =
        substitution_ir_key_at(source, i);
    if(parser_trace::enabled("symbol.linkage")) {
      ostringstream trace;
      trace << "dependent-parameter-merge source index=" << i
            << " key=" << key
            << " ir-kind=" << static_cast<int>(ir_key.kind);
      parser_trace::note("symbol.linkage", string(), trace.str());
    }
    if(substitution_slot_is_template_parameter(key, ir_key)) {
      if(first_template_parameter_index == source.substitution_keys.size()) {
        first_template_parameter_index = i;
      }
      continue;
    }
    if(i < first_template_parameter_index &&
       substitution_slot_is_template_prefix(key, ir_key)) {
      has_template_prefix_before_parameter = true;
    } else if(i > first_template_parameter_index &&
              substitution_slot_is_decay_alias_name(key, ir_key)) {
      has_decay_alias_after_parameter = true;
    }
  }

  bool merged_primary_prefix = false;
  const bool has_template_parameter_key =
      first_template_parameter_index != source.substitution_keys.size();
  for(size_t i = first_new_index; i < source.substitution_keys.size(); ++i) {
    const string & key = source.substitution_keys[i];
    const abi_mangle::SubstitutionKey & ir_key =
        substitution_ir_key_at(source, i);
    if(i < first_template_parameter_index) {
      if(!has_template_parameter_key) {
        if(parser_trace::enabled("symbol.linkage")) {
          parser_trace::note("symbol.linkage",
                             string(),
                             string("dependent-parameter-merge register ") + key);
        }
        register_substitution_slot_if_absent(target, key, ir_key);
      } else if(has_decay_alias_after_parameter) {
        if(substitution_slot_is_template_prefix(key, ir_key)) {
          if(parser_trace::enabled("symbol.linkage")) {
            parser_trace::note("symbol.linkage",
                               string(),
                               string("dependent-parameter-merge register ") + key);
          }
          register_substitution_slot_if_absent(target, key, ir_key);
          merged_primary_prefix = true;
        } else if(!has_template_prefix_before_parameter &&
                  !merged_primary_prefix &&
                  substitution_slot_is_name(key, ir_key)) {
          if(parser_trace::enabled("symbol.linkage")) {
            parser_trace::note("symbol.linkage",
                               string(),
                               string("dependent-parameter-merge register ") + key);
          }
          register_substitution_slot_if_absent(target, key, ir_key);
          merged_primary_prefix = true;
        }
      } else if(substitution_slot_is_template_prefix(key, ir_key) ||
                substitution_slot_is_name(key, ir_key) ||
                substitution_slot_is_member_named_type(key, ir_key)) {
        if(parser_trace::enabled("symbol.linkage")) {
          parser_trace::note("symbol.linkage",
                             string(),
                             string("dependent-parameter-merge register ") + key);
        }
        register_substitution_slot_if_absent(target, key, ir_key);
      }
      continue;
    }
    if(parser_trace::enabled("symbol.linkage")) {
      parser_trace::note("symbol.linkage",
                         string(),
                         string("dependent-parameter-merge register ") + key);
    }
    register_substitution_slot_if_absent(target, key, ir_key);
  }
  if(parser_trace::enabled("symbol.linkage")) {
    ostringstream trace;
    trace << "dependent-parameter-merge end target-size="
          << target->substitution_keys.size();
    for(size_t i = first_new_index; i < target->substitution_keys.size(); ++i) {
      trace << " [" << i << "]=" << target->substitution_keys[i];
    }
    parser_trace::note("symbol.linkage", string(), trace.str());
  }
}

struct MangleIrSubstitutionSink : public abi_mangle::SubstitutionSink
{
  explicit MangleIrSubstitutionSink(MangleSubstitutionState * state) : state(state) {}

  bool emit_substitution(const abi_mangle::SubstitutionKey & key,
                         string & out) override
  {
    return emit_registered_ir_substitution(state, key, out);
  }

  void register_substitution(const abi_mangle::SubstitutionKey & key) override
  {
    register_ir_substitution_key(state, key);
  }

  void register_substitution_owned(abi_mangle::SubstitutionKey key) override
  {
    register_ir_substitution_key_owned(state, std::move(key));
  }

  bool emit_dependent_parameter_type(const abi_mangle::Type & type,
                                     string & out) override
  {
    if(!state) {
      return abi_mangle::emit_type(type, out, this);
    }

    const size_t begin = out.size();
    MangleSubstitutionState parameter_type_state =
        fork_dependent_parameter_substitution_state(state);
    MangleIrSubstitutionSink parameter_type_sink(&parameter_type_state);
    if(!abi_mangle::emit_type(type, out, &parameter_type_sink) ||
       out.size() == begin) {
      out.resize(begin);
      return false;
    }
    merge_dependent_parameter_type_substitutions(parameter_type_state, state);
    return true;
  }

  MangleSubstitutionState * state;
};

static bool emit_function_name_ir(
    const abi_mangle::FunctionEncoding & function,
    MangleSubstitutionState * state,
    string & out)
{
  MangleIrSubstitutionSink sink(state);
  string candidate;
  if(!abi_mangle::emit_function_name_symbol(function, candidate, &sink)) {
    return false;
  }
  out.swap(candidate);
  return true;
}

static bool emit_function_encoding_ir_owned(
    abi_mangle::FunctionEncoding & function,
    MangleSubstitutionState * state,
    string & out)
{
  MangleIrSubstitutionSink sink(state);
  string candidate;
  if(!abi_mangle::emit_function_encoding_owned(function, candidate, &sink)) {
    return false;
  }
  out.swap(candidate);
  return true;
}

static bool emit_function_encoding_ir(
    const abi_mangle::FunctionEncoding & function,
    MangleSubstitutionState * state,
    string & out)
{
  MangleIrSubstitutionSink sink(state);
  string candidate;
  if(!abi_mangle::emit_function_encoding(function, candidate, &sink)) {
    return false;
  }
  out.swap(candidate);
  return true;
}

struct StringMemoEntry
{
  bool occupied = false;
  string key;
  string value;
};

template <size_t N>
static bool lookup_string_memo(array<StringMemoEntry, N> & cache,
                               const string & key,
                               string & out)
{
  const size_t index = hash<string>()(key) & (N - 1);
  const StringMemoEntry & entry = cache[index];
  if(!entry.occupied || entry.key != key) {
    return false;
  }
  out = entry.value;
  return true;
}

template <size_t N>
static void store_string_memo(array<StringMemoEntry, N> & cache,
                              const string & key,
                              const string & value)
{
  const size_t index = hash<string>()(key) & (N - 1);
  StringMemoEntry & entry = cache[index];
  entry.occupied = true;
  entry.key = key;
  entry.value = value;
}

static string canonical_template_argument_text(const string & text)
{
  string out = trim_space(strip_elaborated_type_prefix(text));
  static const string typename_prefix = "typename ";
  if(out.compare(0, typename_prefix.size(), typename_prefix) == 0) {
    out = trim_space(out.substr(typename_prefix.size()));
  }
  return out;
}

static string canonical_component_text(const string & text)
{
  static thread_local array<StringMemoEntry, 4096> cache;
  string cached;
  if(lookup_string_memo(cache, text, cached)) {
    return cached;
  }

  TemplateComponent component;
  if(!parse_template_component(text, component) || component.base_name.empty()) {
    string fallback = trim_space(text);
    store_string_memo(cache, text, fallback);
    return fallback;
  }

  string out = trim_space(component.base_name);
  if(!component.has_template_id) {
    store_string_memo(cache, text, out);
    return out;
  }

  out += '<';
  for(size_t i = 0; i < component.arg_texts.size(); ++i) {
    if(i != 0) {
      out += ", ";
    }
    out += canonical_template_argument_text(component.arg_texts[i]);
  }
  out += '>';
  store_string_memo(cache, text, out);
  return out;
}

static string append_qualified_component_text(const string & prefix,
                                              const string & component)
{
  if(prefix.empty()) {
    return component;
  }
  string out;
  out.reserve(prefix.size() + 2 + component.size());
  out += prefix;
  out += "::";
  out += component;
  return out;
}

static string join_canonical_qualified_parts(const vector<string> & parts, size_t count)
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

static string named_substitution_key(const string & canonical_text)
{
  static thread_local array<StringMemoEntry, 4096> cache;
  string cached;
  if(lookup_string_memo(cache, canonical_text, cached)) {
    return cached;
  }

  const string stripped = trim_elaborated_type_prefix(canonical_text);
  if(stripped.empty()) {
    string empty;
    store_string_memo(cache, canonical_text, empty);
    return empty;
  }

  string out;
  QualifiedName qualified;
  if(semantic_utils::split_qualified_name_text(stripped, qualified) &&
     !qualified.rooted &&
     !qualified.name.empty()) {
    vector<string> parts = qualified.qualifiers;
    parts.push_back(qualified.name);
    vector<string> canonical_parts;
    canonical_parts.reserve(parts.size());
    for(size_t i = 0; i < parts.size(); ++i) {
      canonical_parts.push_back(canonical_component_text(parts[i]));
    }
    out = string("name:") +
          join_canonical_qualified_parts(canonical_parts,
                                         canonical_parts.size());
    store_string_memo(cache, canonical_text, out);
    return out;
  }

  out = string("name:") + canonical_component_text(stripped);
  store_string_memo(cache, canonical_text, out);
  return out;
}

static bool canonicalize_named_substitution_text(const string & text, string & out);
static bool lexical_scope_supports_local_component_emission(const string & text);
static string preferred_named_type_text(const TypePtr & type,
                                        const TypeMangleContext * mangle_ctx);
static bool syntax_name_matches_template_parameter(
    const string & text,
    const TypeMangleContext * mangle_ctx);
static string qualified_name_syntax_key_text(const QualifiedName & name);
static string template_id_syntax_key_text(const TemplateIdSyntax & syntax);
static const semantic_model::Scope * root_scope(const semantic_model::Scope * scope)
{
  while(scope && scope->parent) {
    scope = scope->parent;
  }
  return scope;
}

static bool scope_prefix_text_for_template_decl(const semantic_model::Scope * scope,
                                                string & out)
{
  vector<string> parts;
  for(const semantic_model::Scope * current = scope; current; current = current->parent) {
    if(current->class_info &&
       current->class_info->member_scope.get() == current) {
      string class_prefix = class_output_qualified_name(*current->class_info);
      if(!class_prefix.empty()) {
        if(!parts.empty()) {
          reverse(parts.begin(), parts.end());
          for(size_t i = 0; i < parts.size(); ++i) {
            class_prefix = append_qualified_component_text(class_prefix, parts[i]);
          }
        }
        out = class_prefix;
        return !out.empty();
      }
    }
    if(current->namespace_scope && current->parent && !current->name.empty() &&
       current->name != "<global>") {
      parts.push_back(current->name);
    }
  }
  reverse(parts.begin(), parts.end());
  out = join_canonical_qualified_parts(parts, parts.size());
  return !out.empty();
}

static bool scope_prefix_syntax_for_template_decl(
    const semantic_model::Scope * scope,
    QualifiedName & out)
{
  vector<string> parts;
  for(const semantic_model::Scope * current = scope; current; current = current->parent) {
    if(current->class_info &&
       current->class_info->member_scope.get() == current) {
      out = current->class_info->symbol_qualified_name_syntax;
      if(out.name.empty()) {
        return false;
      }
      reverse(parts.begin(), parts.end());
      for(size_t i = 0; i < parts.size(); ++i) {
        out.qualifiers.push_back(out.name);
        out.name = parts[i];
      }
      return true;
    }
    if(current->namespace_scope && current->parent && !current->name.empty() &&
       current->name != "<global>") {
      parts.push_back(current->name);
    }
  }
  reverse(parts.begin(), parts.end());
  out = QualifiedName();
  if(parts.empty()) {
    return false;
  }
  out.qualifiers.assign(parts.begin(), parts.end() - 1);
  out.name = parts.back();
  return true;
}

static const semantic_model::ClassTemplateDecl *
find_class_template_in_inline_namespace_children(
    const semantic_model::Scope & scope,
    const string & name)
{
  for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
    const semantic_model::Scope & child = *scope.namespace_children[i];
    if(!child.inline_namespace && child.name != "<unnamed>") {
      continue;
    }
    map<string, semantic_model::ClassTemplateDecl *>::const_iterator found =
        child.class_templates.find(name);
    if(found != child.class_templates.end()) {
      return found->second;
    }
    if(const semantic_model::ClassTemplateDecl * nested =
           find_class_template_in_inline_namespace_children(child, name)) {
      return nested;
    }
  }
  return nullptr;
}

static const semantic_model::ClassTemplateDecl *
find_unqualified_class_template_in_mangle_scope(
    const semantic_model::Scope & scope,
    const string & name)
{
  map<string, semantic_model::ClassTemplateDecl *>::const_iterator found =
      scope.class_templates.find(name);
  if(found != scope.class_templates.end()) {
    return found->second;
  }
  if(scope.class_info && scope.class_info->member_scope) {
    found = scope.class_info->member_scope->class_templates.find(name);
    if(found != scope.class_info->member_scope->class_templates.end()) {
      return found->second;
    }
  }
  if(!scope.inline_namespace) {
    if(const semantic_model::ClassTemplateDecl * in_inline =
           find_class_template_in_inline_namespace_children(scope, name)) {
      return in_inline;
    }
  }
  return nullptr;
}

static const semantic_model::AliasTemplateDecl *
find_alias_template_in_inline_namespace_children(
    const semantic_model::Scope & scope,
    const string & name)
{
  for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
    const semantic_model::Scope & child = *scope.namespace_children[i];
    if(!child.inline_namespace && child.name != "<unnamed>") {
      continue;
    }
    map<string, semantic_model::AliasTemplateDecl *>::const_iterator found =
        child.alias_templates.find(name);
    if(found != child.alias_templates.end()) {
      return found->second;
    }
    if(const semantic_model::AliasTemplateDecl * nested =
           find_alias_template_in_inline_namespace_children(child, name)) {
      return nested;
    }
  }
  return nullptr;
}

static const semantic_model::AliasTemplateDecl *
find_unqualified_alias_template_in_mangle_scope(
    const semantic_model::Scope & scope,
    const string & name)
{
  map<string, semantic_model::AliasTemplateDecl *>::const_iterator found =
      scope.alias_templates.find(name);
  if(found != scope.alias_templates.end()) {
    return found->second;
  }
  if(scope.class_info && scope.class_info->member_scope) {
    found = scope.class_info->member_scope->alias_templates.find(name);
    if(found != scope.class_info->member_scope->alias_templates.end()) {
      return found->second;
    }
  }
  if(!scope.inline_namespace) {
    if(const semantic_model::AliasTemplateDecl * in_inline =
           find_alias_template_in_inline_namespace_children(scope, name)) {
      return in_inline;
    }
  }
  return nullptr;
}

static const semantic_model::ClassTemplateDecl * lookup_class_template_for_template_id_syntax(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx || !mangle_ctx->lookup_scope) {
    return nullptr;
  }

  const string base_name =
      strip_leading_template_disambiguator(syntax.name.name);
  if(base_name.empty()) {
    return nullptr;
  }

  if(syntax.name.qualifiers.empty()) {
    for(const semantic_model::Scope * scope = mangle_ctx->lookup_scope;
        scope;
        scope = scope->parent) {
      if(const semantic_model::ClassTemplateDecl * found =
             find_unqualified_class_template_in_mangle_scope(*scope, base_name)) {
        return found;
      }
    }
    if(!mangle_ctx->lexical_scope_syntax.name.empty()) {
      const QualifiedName & lexical = mangle_ctx->lexical_scope_syntax;
      if(!lexical.rooted) {
        const semantic_model::Scope * scope = root_scope(mangle_ctx->lookup_scope);
        vector<string> parts = lexical.qualifiers;
        parts.push_back(lexical.name);
        size_t part_index = 0;
        if(scope &&
           scope->namespace_scope &&
           !scope->name.empty() &&
           !parts.empty() &&
           scope->name == parts[0]) {
          part_index = 1;
        }
        for(size_t i = part_index; scope && i < parts.size(); ++i) {
          map<string, semantic_model::Scope *>::const_iterator ns_found =
              scope->namespace_bindings.find(parts[i]);
          scope = ns_found == scope->namespace_bindings.end() ? nullptr : ns_found->second;
        }
        if(scope) {
          if(const semantic_model::ClassTemplateDecl * found =
                 find_unqualified_class_template_in_mangle_scope(*scope, base_name)) {
            return found;
          }
        }
      }
    }
    return nullptr;
  }

  const semantic_model::Scope * scope = root_scope(mangle_ctx->lookup_scope);
  for(size_t i = 0; scope && i < syntax.name.qualifiers.size(); ++i) {
    const string qualifier =
        semantic_utils::strip_trailing_top_level_template_arguments(
            trim_space(syntax.name.qualifiers[i]));
    map<string, semantic_model::Scope *>::const_iterator found =
        scope->namespace_bindings.find(qualifier);
    scope = found == scope->namespace_bindings.end() ? nullptr : found->second;
  }
  if(!scope) {
    return nullptr;
  }
  return find_unqualified_class_template_in_mangle_scope(*scope, base_name);
}

static bool qualify_template_id_syntax_from_lookup(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx,
    QualifiedName & out)
{
  if(!syntax.name.qualifiers.empty()) {
    return false;
  }
  if(syntax_name_matches_template_parameter(
         strip_leading_template_disambiguator(syntax.name.name),
         mangle_ctx)) {
    return false;
  }
  const semantic_model::ClassTemplateDecl * class_template =
      lookup_class_template_for_template_id_syntax(syntax, mangle_ctx);
  if(!class_template) {
    return false;
  }
  QualifiedName qualified_prefix;
  if(!scope_prefix_syntax_for_template_decl(class_template->declaring_scope,
                                            qualified_prefix) ||
     qualified_prefix.rooted) {
    return false;
  }
  out = QualifiedName();
  out.rooted = false;
  out.qualifiers = qualified_prefix.qualifiers;
  out.qualifiers.push_back(qualified_prefix.name);
  out.name = strip_leading_template_disambiguator(syntax.name.name);
  return !out.name.empty();
}

static bool direct_std_standard_substitutions_enabled(const TypeMangleContext * mangle_ctx)
{
  return !mangle_ctx || mangle_ctx->allow_direct_std_standard_substitutions;
}

static bool canonicalize_named_substitution_text(const string & text, string & out)
{
  static thread_local array<StringMemoEntry, 4096> cache;
  string cached;
  if(lookup_string_memo(cache, text, cached)) {
    out = cached;
    return !out.empty();
  }

  const string stripped = trim_elaborated_type_prefix(text);
  if(stripped.empty()) {
    out.clear();
    store_string_memo(cache, text, out);
    return false;
  }

  QualifiedName qualified;
  if(!semantic_utils::split_qualified_name_text(stripped, qualified) ||
     qualified.rooted ||
     qualified.name.empty()) {
    out = stripped;
    store_string_memo(cache, text, out);
    return true;
  }

  vector<string> parts = qualified.qualifiers;
  parts.push_back(qualified.name);
  vector<string> canonical_parts;
  canonical_parts.reserve(parts.size());
  for(size_t i = 0; i < parts.size(); ++i) {
    canonical_parts.push_back(canonical_component_text(parts[i]));
  }
  out = join_canonical_qualified_parts(canonical_parts, canonical_parts.size());
  store_string_memo(cache, text, out);
  return !out.empty();
}

static bool template_argument_is_char_type(const TemplateArgument & argument)
{
  if(argument.kind != TemplateArgument::TA_TYPE) {
    return false;
  }
  TypePtr base = strip_top_level_cv(argument.type);
  return base && base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_CHAR;
}

static bool type_is_std_class_template_specialization(
    const TypePtr & type,
    const string & template_name,
    size_t argument_count,
    shared_ptr<const ClassTemplateSpecializationMangleInfo> * info_out = nullptr)
{
  shared_ptr<const ClassTemplateSpecializationMangleInfo> info =
      named_type_class_template_specialization_mangle_info_const(type);
  if(!info) {
    return false;
  }

  string canonical_scope;
  if(!canonicalize_named_substitution_text(info->template_scope_prefix,
                                           canonical_scope) ||
     canonical_scope != "std" ||
     trim_space(info->template_name) != template_name ||
     info->arguments.size() != argument_count) {
    return false;
  }

  if(info_out) {
    *info_out = info;
  }
  return true;
}

static bool template_argument_is_std_char_traits_char(
    const TemplateArgument & argument)
{
  if(argument.kind != TemplateArgument::TA_TYPE) {
    return false;
  }
  shared_ptr<const ClassTemplateSpecializationMangleInfo> info;
  return type_is_std_class_template_specialization(argument.type,
                                                  "char_traits",
                                                  1,
                                                  &info) &&
         template_argument_is_char_type(info->arguments[0]);
}

static bool template_argument_is_std_allocator_char(
    const TemplateArgument & argument)
{
  if(argument.kind != TemplateArgument::TA_TYPE) {
    return false;
  }
  shared_ptr<const ClassTemplateSpecializationMangleInfo> info;
  return type_is_std_class_template_specialization(argument.type,
                                                  "allocator",
                                                  1,
                                                  &info) &&
         template_argument_is_char_type(info->arguments[0]);
}

static bool structured_arguments_match_char_stream(
    const vector<TemplateArgument> & arguments)
{
  return arguments.size() == 2 &&
         template_argument_is_char_type(arguments[0]) &&
         template_argument_is_std_char_traits_char(arguments[1]);
}

static bool structured_arguments_match_basic_string(
    const vector<TemplateArgument> & arguments)
{
  return arguments.size() == 3 &&
         template_argument_is_char_type(arguments[0]) &&
         template_argument_is_std_char_traits_char(arguments[1]) &&
         template_argument_is_std_allocator_char(arguments[2]);
}

static bool structured_std_standard_substitution_for_template_component(
    const string & component,
    const vector<TemplateArgument> & arguments,
    const string & canonical_prefix,
    const TypeMangleContext * mangle_ctx,
    string & code,
    bool & substitution_includes_arguments)
{
  code.clear();
  substitution_includes_arguments = false;
  if(canonical_prefix != "std") {
    return false;
  }

  if(component == "allocator") {
    code = "Sa";
    return true;
  }

  if(component == "basic_string") {
    if(direct_std_standard_substitutions_enabled(mangle_ctx) &&
       structured_arguments_match_basic_string(arguments)) {
      code = "Ss";
      substitution_includes_arguments = true;
      return true;
    }
    code = "Sb";
    return true;
  }

  if(!direct_std_standard_substitutions_enabled(mangle_ctx)) {
    return false;
  }

  if(component == "basic_istream" &&
     structured_arguments_match_char_stream(arguments)) {
    code = "Si";
    substitution_includes_arguments = true;
    return true;
  }
  if(component == "basic_ostream" &&
     structured_arguments_match_char_stream(arguments)) {
    code = "So";
    substitution_includes_arguments = true;
    return true;
  }
  if(component == "basic_iostream" &&
     structured_arguments_match_char_stream(arguments)) {
    code = "Sd";
    substitution_includes_arguments = true;
    return true;
  }
  return false;
}

static bool lexical_scope_supports_local_component_emission(const string & text)
{
  return text.find("(anonymous namespace)") != string::npos ||
         text.find("__local_") != string::npos;
}

static bool qualified_type_text_lexical_lookup_prefix(
    const QualifiedName & qualified,
    const TypeMangleContext * mangle_ctx,
    QualifiedName & out)
{
  out = QualifiedName();
  if(!mangle_ctx ||
     !mangle_ctx->lookup_scope ||
     mangle_ctx->lexical_scope.empty() ||
     qualified.qualifiers.empty()) {
    return false;
  }

  const string first_qualifier = trim_space(qualified.qualifiers[0]);
  const string first =
      semantic_utils::strip_trailing_top_level_template_arguments(first_qualifier);
  if(first.empty() || first == "std") {
    return false;
  }
  if(first != first_qualifier) {
    return false;
  }

  for(const semantic_model::Scope * scope = mangle_ctx->lookup_scope;
      scope;
      scope = scope->parent) {
    map<string, semantic_model::Scope *>::const_iterator ns_found =
        scope->namespace_bindings.find(first);
    if(ns_found != scope->namespace_bindings.end() && ns_found->second) {
      QualifiedName full_scope;
      if(!scope_prefix_syntax_for_template_decl(ns_found->second, full_scope) ||
         full_scope.qualifiers.empty()) {
        return false;
      }
      out.rooted = full_scope.rooted;
      out.qualifiers.assign(full_scope.qualifiers.begin(),
                            full_scope.qualifiers.end() - 1);
      out.name = full_scope.qualifiers.back();
      return true;
    }

    if(scope->named_types.find(first) != scope->named_types.end() ||
       scope->class_templates.find(first) != scope->class_templates.end() ||
       scope->alias_templates.find(first) != scope->alias_templates.end()) {
      return scope_prefix_syntax_for_template_decl(scope, out) &&
             !out.name.empty();
    }
  }

  if(lexical_scope_supports_local_component_emission(mangle_ctx->lexical_scope) &&
     lexical_scope_supports_local_component_emission(first)) {
    out = mangle_ctx->lexical_scope_syntax;
    return !out.name.empty();
  }
  return false;
}

static bool template_argument_is_self_type_parameter(
    const TemplateArgument & argument,
    const TemplateParameterInfo & parameter,
    const TypePtr & matched_type,
    const string & matched_display);

static bool emit_template_parameter_index(size_t index,
                                          const TemplateParameterInfo & parameter,
                                          string & out,
                                          MangleSubstitutionState * state,
                                          bool register_type_substitution,
                                          const vector<TemplateParameterInfo> * parameters = nullptr);
static bool emit_template_parameter_expression_index(
    size_t index,
    const TemplateParameterInfo & parameter,
    string & out,
    MangleSubstitutionState * state,
    bool register_type_substitution);

static bool try_mangle_template_parameter_text(const string & text,
                                               const TypeMangleContext * mangle_ctx,
                                               string & out,
                                               MangleSubstitutionState * state,
                                               bool register_type_substitution = true,
                                               bool force_pack_expansion = false,
                                               bool register_new_type_substitution = true)
{
  if(!mangle_ctx) {
    return false;
  }

  string stripped = trim_elaborated_type_prefix(text);
  bool pack_expansion = false;
  if(stripped.size() >= 3 &&
     stripped.compare(stripped.size() - 3, 3, "...") == 0) {
    pack_expansion = true;
    stripped = trim_space(stripped.substr(0, stripped.size() - 3));
  }
  if(mangle_ctx->template_parameters &&
     mangle_ctx->template_parameters->parameters) {
    const vector<TemplateParameterInfo> * parameters =
        mangle_ctx->template_parameters->parameters;
    for(size_t i = 0; i < parameters->size(); ++i) {
      const TemplateParameterInfo & param = (*parameters)[i];
      if((param.kind != TemplateParameterInfo::TP_TYPE &&
          param.kind != TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) ||
         param.name.empty()) {
        continue;
      }
      const bool matches_type_parameter =
          param.kind == TemplateParameterInfo::TP_TYPE &&
          text_matches_type_parameter_name(stripped, param.name);
      const bool matches_template_template_parameter =
          param.kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE &&
          stripped == param.name;
      if(matches_type_parameter || matches_template_template_parameter) {
        if(pack_expansion || force_pack_expansion) {
          if(!param.parameter_pack) {
            continue;
          }
          out += "Dp";
        }
        if(mangle_ctx->prefer_template_argument_values &&
           !param.parameter_pack &&
           mangle_ctx->template_arguments &&
           i < mangle_ctx->template_arguments->size()) {
          return try_mangle_template_argument_impl(
              (*mangle_ctx->template_arguments)[i], &param, out, mangle_ctx, state);
        }
        const string param_key =
            template_parameter_type_substitution_key(parameters, i, param);
        if(register_type_substitution &&
           emit_registered_substitution(state, param_key, out)) {
          return true;
        }
        out += 'T';
        if(i > 0) {
          out += to_string(i - 1);
        }
        out += '_';
        if(register_type_substitution &&
           !mangle_ctx->suppress_template_parameter_type_registration) {
          if(register_new_type_substitution) {
            register_substitution_key(state, param_key);
          }
          if(pack_expansion) {
            if(register_new_type_substitution) {
              register_substitution_key(
                  state,
                  string("template-arg-pack(") + param_key + ")");
            }
          }
        }
        return true;
      }
    }
  }

  if(mangle_ctx->owner_template_parameters &&
     mangle_ctx->owner_template_arguments) {
    const vector<TemplateParameterInfo> & parameters =
        *mangle_ctx->owner_template_parameters;
    const vector<TemplateArgument> & arguments =
        *mangle_ctx->owner_template_arguments;
    for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
      const TemplateParameterInfo & param = parameters[i];
      if((param.kind != TemplateParameterInfo::TP_TYPE &&
          param.kind != TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) ||
         param.name.empty()) {
        continue;
      }
      const bool matches_type_parameter =
          param.kind == TemplateParameterInfo::TP_TYPE &&
          text_matches_type_parameter_name(stripped, param.name);
      const bool matches_template_template_parameter =
          param.kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE &&
          stripped == param.name;
      if(matches_type_parameter || matches_template_template_parameter) {
        if(owner_template_argument_index_is_suppressed(mangle_ctx, i)) {
          return emit_template_parameter_index(
              i, param, out, state, register_type_substitution, &parameters);
        }
        if(pack_expansion && !param.parameter_pack) {
          continue;
        }
        if(param.kind == TemplateParameterInfo::TP_TYPE &&
           template_argument_is_self_type_parameter(
               arguments[i], param, TypePtr(), stripped)) {
          return emit_template_parameter_index(
              i, param, out, state, register_type_substitution, &parameters);
        }
        if(param.kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE &&
           arguments[i].kind == TemplateArgument::TA_CLASS_TEMPLATE &&
           trim_space(arguments[i].text) == param.name) {
          return emit_template_parameter_index(
              i, param, out, state, register_type_substitution, &parameters);
        }
        if(pack_expansion) {
          return false;
        }
        TypeMangleContext owner_arg_ctx_storage;
        return try_mangle_template_argument_impl(
            arguments[i],
            &param,
            out,
            suppress_owner_template_argument_index(
                mangle_ctx, i, owner_arg_ctx_storage),
            state);
      }
    }
  }

  return false;
}

static bool template_argument_is_self_type_parameter(
    const TemplateArgument & argument,
    const TemplateParameterInfo & parameter,
    const TypePtr & matched_type,
    const string & matched_display)
{
  if(argument.kind != TemplateArgument::TA_TYPE) {
    return false;
  }
  TypePtr argument_base = strip_top_level_cv(argument.type);
  if(argument_base) {
    if(argument_base->kind == Type::TK_NAMED &&
       argument_base->named_semantic_kind == Type::NSK_TEMPLATE_PARAMETER) {
      if(!parameter.placeholder_key.empty() &&
         argument_base->named_key == parameter.placeholder_key) {
        return true;
      }
      const string argument_display =
          trim_elaborated_type_prefix(argument_base->named_display);
      if(!parameter.name.empty() &&
         text_matches_type_parameter_name(argument_display, parameter.name)) {
        return true;
      }
    }
    if(!matched_type) {
      return false;
    }
    return argument_base.get() == matched_type.get() ||
           (argument_base->kind == Type::TK_NAMED &&
            argument_base->named_key == matched_type->named_key &&
            trim_elaborated_type_prefix(
                argument_base->named_display) == matched_display);
  }

  const string argument_text =
      trim_elaborated_type_prefix(argument.text);
  if(!parameter.name.empty() &&
     (text_matches_type_parameter_name(argument_text, parameter.name) ||
      (parameter.parameter_pack && argument_text == parameter.name + "..."))) {
    return true;
  }
  return false;
}

static bool emit_template_parameter_index(size_t index,
                                          const TemplateParameterInfo & parameter,
                                          string & out,
                                          MangleSubstitutionState * state,
                                          bool register_type_substitution,
                                          const vector<TemplateParameterInfo> * parameters)
{
  if(parameter.parameter_pack) {
    out += "Dp";
  }
  const string param_key =
      template_parameter_type_substitution_key(parameters, index, parameter);
  if(register_type_substitution &&
     emit_registered_substitution(state, param_key, out)) {
    return true;
  }
  out += 'T';
  if(index > 0) {
    out += to_string(index - 1);
  }
  out += '_';
  if(register_type_substitution) {
    register_substitution_key(state, param_key);
    if(parameter.parameter_pack) {
      register_substitution_key(
          state,
          string("template-arg-pack(") + param_key + ")");
    }
  }
  return true;
}

static bool emit_template_parameter_expression_index(
    size_t index,
    const TemplateParameterInfo & parameter,
    string & out,
    MangleSubstitutionState * state,
    bool register_type_substitution)
{
  const string param_key = string("type:tparam(") + parameter.name + ")";
  if(register_type_substitution &&
     emit_registered_substitution(state, param_key, out)) {
    return true;
  }
  out += 'T';
  if(index > 0) {
    out += to_string(index - 1);
  }
  out += '_';
  if(register_type_substitution) {
    register_substitution_key(state, param_key);
  }
  return true;
}

static TypePtr lookup_scope_named_type_for_mangling(
    const string & text,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx || !mangle_ctx->lookup_scope) {
    return TypePtr();
  }

  const string stripped = trim_elaborated_type_prefix(text);
  if(stripped.empty() ||
     stripped.find("::") != string::npos ||
     stripped.find('<') != string::npos) {
    return TypePtr();
  }

  for(const semantic_model::Scope * scope = mangle_ctx->lookup_scope;
      scope;
      scope = scope->parent) {
    auto found = scope->named_types.find(stripped);
    if(found == scope->named_types.end() || !found->second) {
      continue;
    }
    if(scope->class_info &&
       scope->class_info->type &&
       scope->class_info->type == found->second) {
      return found->second;
    }
    if(trim_elaborated_type_prefix(found->second->named_display) == stripped &&
       trim_elaborated_type_prefix(found->second->named_key) == stripped) {
      return TypePtr();
    }
    return found->second;
  }

  return TypePtr();
}

static const semantic_model::Scope * lookup_namespace_scope_for_mangling(
    const QualifiedName & qualified,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx || !mangle_ctx->lookup_scope || qualified.rooted) {
    return nullptr;
  }

  const semantic_model::Scope * scope = root_scope(mangle_ctx->lookup_scope);
  if(!scope) {
    return nullptr;
  }

  size_t index = 0;
  if(scope->namespace_scope &&
     !scope->name.empty() &&
     !qualified.qualifiers.empty() &&
     scope->name == qualified.qualifiers[0]) {
    index = 1;
  }

  for(; index < qualified.qualifiers.size(); ++index) {
    const string qualifier =
        semantic_utils::strip_trailing_top_level_template_arguments(
            trim_space(qualified.qualifiers[index]));
    if(qualifier.empty() || qualifier != qualified.qualifiers[index]) {
      return nullptr;
    }
    map<string, semantic_model::Scope *>::const_iterator found =
        scope->namespace_bindings.find(qualifier);
    if(found == scope->namespace_bindings.end() || !found->second) {
      return nullptr;
    }
    scope = found->second;
  }

  return scope;
}

static TypePtr lookup_qualified_named_type_for_mangling(
    const QualifiedName & qualified,
    const TypeMangleContext * mangle_ctx)
{
  if(qualified.name.empty()) {
    return TypePtr();
  }

  const semantic_model::Scope * scope =
      lookup_namespace_scope_for_mangling(qualified, mangle_ctx);
  if(!scope) {
    return TypePtr();
  }

  auto found =
      scope->named_types.find(qualified.name);
  if(found == scope->named_types.end() || !found->second) {
    return TypePtr();
  }
  return found->second;
}

static bool text_mentions_template_mangle_parameter(
    const string & text,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx || !mangle_ctx->template_parameters ||
     !mangle_ctx->template_parameters->parameters) {
    if(!mangle_ctx || !mangle_ctx->owner_template_parameters) {
      return false;
    }
  }
  const auto mentions = [&](const vector<TemplateParameterInfo> & parameters) -> bool
  {
    for(size_t i = 0; i < parameters.size(); ++i) {
      const TemplateParameterInfo & parameter = parameters[i];
      if(contains_identifier_token(text, parameter.name) ||
         contains_identifier_token(text, parameter.placeholder_key)) {
        return true;
      }
      for(size_t j = 0; j < parameter.alternate_names.size(); ++j) {
        if(contains_identifier_token(text, parameter.alternate_names[j])) {
          return true;
        }
      }
    }
    return false;
  };
  if(mangle_ctx->template_parameters &&
     mangle_ctx->template_parameters->parameters &&
     mentions(*mangle_ctx->template_parameters->parameters)) {
    return true;
  }
  if(mangle_ctx->owner_template_parameters &&
     mentions(*mangle_ctx->owner_template_parameters)) {
    return true;
  }
  return false;
}

static bool text_mentions_direct_template_mangle_parameter(
    const string & text,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx ||
     !mangle_ctx->template_parameters ||
     !mangle_ctx->template_parameters->parameters) {
    return false;
  }

  const vector<TemplateParameterInfo> & parameters =
      *mangle_ctx->template_parameters->parameters;
  for(size_t i = 0; i < parameters.size(); ++i) {
    const TemplateParameterInfo & parameter = parameters[i];
    if(contains_identifier_token(text, parameter.name) ||
       contains_identifier_token(text, parameter.placeholder_key)) {
      return true;
    }
    for(size_t j = 0; j < parameter.alternate_names.size(); ++j) {
      if(contains_identifier_token(text, parameter.alternate_names[j])) {
        return true;
      }
    }
  }
  return false;
}

static bool template_parameter_matches_named_type_text(
    const TemplateParameterInfo & parameter,
    const string & text)
{
  if(text.empty()) {
    return false;
  }
  const string stripped = trim_elaborated_type_prefix(text);
  if(stripped.empty()) {
    return false;
  }
  if(!parameter.name.empty() &&
     (text_matches_type_parameter_name(stripped, parameter.name) ||
      contains_identifier_token(stripped, parameter.name))) {
    return true;
  }
  if(!parameter.placeholder_key.empty() &&
     (stripped == parameter.placeholder_key ||
      contains_identifier_token(stripped, parameter.placeholder_key))) {
    return true;
  }
  for(size_t i = 0; i < parameter.alternate_names.size(); ++i) {
    if(stripped == parameter.alternate_names[i] ||
       contains_identifier_token(stripped, parameter.alternate_names[i])) {
      return true;
    }
  }
  return false;
}

static bool template_parameter_prefix_matches(
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateParameterInfo> & prefix)
{
  if(parameters.size() < prefix.size()) {
    return false;
  }
  for(size_t i = 0; i < prefix.size(); ++i) {
    if(parameters[i].kind != prefix[i].kind ||
       parameters[i].name != prefix[i].name ||
       parameters[i].parameter_pack != prefix[i].parameter_pack) {
      return false;
    }
  }
  return true;
}

static bool template_parameters_have_matching_type_name(
    const vector<TemplateParameterInfo> * parameters,
    const string & stripped)
{
  if(!parameters) {
    return false;
  }
  for(size_t i = 0; i < parameters->size(); ++i) {
    const TemplateParameterInfo & parameter = (*parameters)[i];
    if(parameter.kind == TemplateParameterInfo::TP_TYPE &&
       !parameter.name.empty() &&
       text_matches_type_parameter_name(stripped, parameter.name)) {
      return true;
    }
  }
  return false;
}

static size_t function_template_parameter_slice_begin(
    const vector<TemplateParameterInfo> * parameters,
    const vector<TemplateParameterInfo> * owner_parameters)
{
  if(!parameters || !owner_parameters || owner_parameters->empty()) {
    return 0;
  }
  return template_parameter_prefix_matches(*parameters, *owner_parameters) ?
      owner_parameters->size() :
      0;
}

static bool type_mentions_template_parameter_slice(
    const TypePtr & type,
    const vector<TemplateParameterInfo> * parameters,
    size_t begin)
{
  if(!type || !parameters || begin >= parameters->size()) {
    return false;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  switch(base->kind) {
  case Type::TK_NAMED:
  {
    const string selected = selected_named_type_text(base);
    const string display = trim_elaborated_type_prefix(base->named_display);
    const string key = trim_elaborated_type_prefix(base->named_key);
    for(size_t i = begin; i < parameters->size(); ++i) {
      const TemplateParameterInfo & parameter = (*parameters)[i];
      if(template_parameter_matches_named_type_text(parameter, selected) ||
         template_parameter_matches_named_type_text(parameter, display) ||
         template_parameter_matches_named_type_text(parameter, key)) {
        return true;
      }
    }
    return false;
  }

  case Type::TK_CV:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    return type_mentions_template_parameter_slice(base->inner, parameters, begin);

  case Type::TK_MEMBER_POINTER:
    return type_mentions_template_parameter_slice(base->owner, parameters, begin) ||
           type_mentions_template_parameter_slice(base->inner, parameters, begin);

  case Type::TK_FUNCTION:
    if(type_mentions_template_parameter_slice(base->inner, parameters, begin)) {
      return true;
    }
    for(size_t i = 0; i < base->params.size(); ++i) {
      if(type_mentions_template_parameter_slice(base->params[i], parameters, begin)) {
        return true;
      }
    }
    return false;

  default:
    return false;
  }
}

static bool type_mentions_function_template_parameter_slice(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx ||
     !mangle_ctx->template_parameters ||
     !mangle_ctx->template_parameters->parameters) {
    return false;
  }
  const vector<TemplateParameterInfo> * parameters =
      mangle_ctx->template_parameters->parameters;
  const size_t begin =
      function_template_parameter_slice_begin(parameters,
                                             mangle_ctx->owner_template_parameters);
  return type_mentions_template_parameter_slice(type, parameters, begin);
}

static void throw_unstructured_dependent_text_mangling(const string & kind,
                                                       const string & text)
{
  throw logic_error("unstructured dependent " + kind +
                    " text reached ABI mangler: " + trim_space(text));
}

static string ast_node_diagnostic_label(const CppAstNode & node)
{
  const string text = trim_space(node.value);
  return text.empty() ? string(cppast_kind_text(node.kind)) : text;
}

static bool syntax_name_matches_template_parameter(
    const string & text,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx) {
    return false;
  }

  const string stripped = trim_elaborated_type_prefix(text);
  const auto matches = [&](const vector<TemplateParameterInfo> & parameters) -> bool
  {
    for(size_t i = 0; i < parameters.size(); ++i) {
      const TemplateParameterInfo & parameter = parameters[i];
      if(template_parameter_identifier_matches(parameter, stripped)) {
        return true;
      }
    }
    return false;
  };

  return (mangle_ctx->template_parameters &&
          mangle_ctx->template_parameters->parameters &&
          matches(*mangle_ctx->template_parameters->parameters)) ||
         (mangle_ctx->owner_template_parameters &&
          matches(*mangle_ctx->owner_template_parameters));
}

static bool try_find_template_template_parameter_index(
    const string & text,
    const TypeMangleContext * mangle_ctx,
    size_t & index,
    const TemplateParameterInfo *& parameter)
{
  if(!mangle_ctx) {
    return false;
  }

  const string stripped = trim_elaborated_type_prefix(text);
  const auto matches_name =
      [&](const TemplateParameterInfo & candidate) -> bool
      {
        if(candidate.kind != TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
          return false;
        }
        if(stripped == candidate.name ||
           stripped == candidate.placeholder_key) {
          return true;
        }
        for(size_t i = 0; i < candidate.alternate_names.size(); ++i) {
          if(stripped == candidate.alternate_names[i]) {
            return true;
          }
        }
        return false;
      };
  const auto find_in =
      [&](const vector<TemplateParameterInfo> * candidates) -> bool
      {
        if(!candidates) {
          return false;
        }
        for(size_t i = 0; i < candidates->size(); ++i) {
          if(matches_name((*candidates)[i])) {
            index = i;
            parameter = &(*candidates)[i];
            return true;
          }
        }
        return false;
      };

  if(mangle_ctx->template_parameters &&
     find_in(mangle_ctx->template_parameters->parameters)) {
    return true;
  }
  return find_in(mangle_ctx->owner_template_parameters);
}

static bool text_mentions_template_parameter_name_only(
    const string & text,
    const TypeMangleContext * mangle_ctx)
{
  const auto mentions = [&](const vector<TemplateParameterInfo> & parameters) -> bool
  {
    for(size_t i = 0; i < parameters.size(); ++i) {
      if(!parameters[i].name.empty() &&
         contains_identifier_token(text, parameters[i].name)) {
        return true;
      }
    }
    return false;
  };
  return (mangle_ctx &&
          mangle_ctx->template_parameters &&
          mangle_ctx->template_parameters->parameters &&
          mentions(*mangle_ctx->template_parameters->parameters)) ||
         (mangle_ctx &&
          mangle_ctx->owner_template_parameters &&
          mentions(*mangle_ctx->owner_template_parameters));
}

static bool ast_node_mentions_template_parameter_name_only(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx)
{
  if(text_mentions_template_parameter_name_only(node.value, mangle_ctx)) {
    return true;
  }
  const TemplateIdSyntax * template_id = cppast_template_id_syntax(node);
  if(template_id &&
     text_mentions_template_parameter_name_only(
         template_id_syntax_key_text(*template_id), mangle_ctx)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(text_mentions_template_parameter_name_only(
           template_id_syntax_key_text(node.qualifier_template_id_syntaxes[i]),
           mangle_ctx)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_mentions_template_parameter_name_only(node.children[i], mangle_ctx)) {
      return true;
    }
  }
  return false;
}

static bool template_argument_syntax_mentions_template_parameter(
    const TemplateArgumentSyntax & syntax,
    const TypeMangleContext * mangle_ctx);

static bool template_id_syntax_mentions_template_parameter(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(syntax_name_matches_template_parameter(
         strip_leading_template_disambiguator(syntax.name.name),
         mangle_ctx)) {
    return true;
  }
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_mentions_template_parameter(
           syntax.argument_syntaxes[i], mangle_ctx)) {
      return true;
    }
  }
  return false;
}

static bool ast_node_mentions_template_parameter(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx)
{
  if(syntax_name_matches_template_parameter(node.value, mangle_ctx) ||
     text_mentions_template_mangle_parameter(node.value, mangle_ctx)) {
    return true;
  }
  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
    if(template_id_syntax_mentions_template_parameter(*template_id, mangle_ctx)) {
      return true;
    }
  }
  if(node.conversion_type_id_syntax &&
     ast_node_mentions_template_parameter(*node.conversion_type_id_syntax,
                                          mangle_ctx)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(ast_node_mentions_template_parameter(node.qualifier_type_syntaxes[i],
                                            mangle_ctx)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    if(ast_node_mentions_template_parameter(node.exception_type_id_syntaxes[i],
                                            mangle_ctx)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_mentions_template_parameter(
           node.qualifier_template_id_syntaxes[i], mangle_ctx)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_mentions_template_parameter(node.children[i], mangle_ctx)) {
      return true;
    }
  }
  return false;
}

static bool ast_node_mentions_direct_template_parameter(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx) {
    return false;
  }
  TypeMangleContext direct_ctx = *mangle_ctx;
  direct_ctx.owner_template_parameters = nullptr;
  direct_ctx.owner_template_arguments = nullptr;
  return ast_node_mentions_template_parameter(node, &direct_ctx);
}

static bool function_parameter_index_for_name(const string & raw_name,
                                              const TypeMangleContext * mangle_ctx,
                                              size_t & out_index)
{
  if(!mangle_ctx || !mangle_ctx->function_parameters) {
    return false;
  }
  const string name = trim_space(raw_name);
  if(name.empty()) {
    return false;
  }
  for(size_t i = 0; i < mangle_ctx->function_parameters->size(); ++i) {
    if((*mangle_ctx->function_parameters)[i].name == name) {
      out_index = i;
      return true;
    }
  }
  return false;
}

static bool ast_node_mentions_function_parameter(const CppAstNode & node,
                                                 const TypeMangleContext * mangle_ctx)
{
  size_t ignored_index = 0;
  if((node.kind == CppAstKind::id_expression ||
      node.kind == CppAstKind::identifier) &&
     function_parameter_index_for_name(node.value, mangle_ctx, ignored_index)) {
    return true;
  }
  if(node.conversion_type_id_syntax &&
     ast_node_mentions_function_parameter(*node.conversion_type_id_syntax,
                                          mangle_ctx)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(ast_node_mentions_function_parameter(node.qualifier_type_syntaxes[i],
                                            mangle_ctx)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    if(ast_node_mentions_function_parameter(node.exception_type_id_syntaxes[i],
                                            mangle_ctx)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_mentions_function_parameter(node.children[i], mangle_ctx)) {
      return true;
    }
  }
  return false;
}

static bool declarator_type_syntax_mentions_function_parameter(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx)
{
  if(node.kind == CppAstKind::identifier) {
    return false;
  }
  size_t ignored_index = 0;
  if(node.kind == CppAstKind::id_expression &&
     function_parameter_index_for_name(node.value, mangle_ctx, ignored_index)) {
    return true;
  }
  if(node.conversion_type_id_syntax &&
     ast_node_mentions_function_parameter(*node.conversion_type_id_syntax,
                                          mangle_ctx)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(ast_node_mentions_function_parameter(node.qualifier_type_syntaxes[i],
                                            mangle_ctx)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    if(ast_node_mentions_function_parameter(node.exception_type_id_syntaxes[i],
                                            mangle_ctx)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(declarator_type_syntax_mentions_function_parameter(node.children[i],
                                                          mangle_ctx)) {
      return true;
    }
  }
  return false;
}

static bool parameter_declaration_type_mentions_function_parameter(
    const CppAstNode & declaration,
    const TypeMangleContext * mangle_ctx)
{
  if(declaration.kind != CppAstKind::parameter_declaration) {
    return ast_node_mentions_function_parameter(declaration, mangle_ctx);
  }
  for(size_t i = 0; i < declaration.children.size(); ++i) {
    const CppAstNode & child = declaration.children[i];
    if(child.kind == CppAstKind::decl_specifier_seq ||
       child.kind == CppAstKind::type_specifier_seq) {
      if(ast_node_mentions_function_parameter(child, mangle_ctx)) {
        return true;
      }
    } else if(child.kind == CppAstKind::declarator ||
              child.kind == CppAstKind::abstract_declarator) {
      if(declarator_type_syntax_mentions_function_parameter(child, mangle_ctx)) {
        return true;
      }
    }
  }
  return false;
}

static bool template_argument_syntax_mentions_template_parameter(
    const TemplateArgumentSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(!syntax.text.empty() &&
     text_mentions_template_mangle_parameter(syntax.text, mangle_ctx)) {
    return true;
  }
  if(syntax.template_id &&
     template_id_syntax_mentions_template_parameter(*syntax.template_id, mangle_ctx)) {
    return true;
  }
  if(syntax.type_id &&
     ast_node_mentions_template_parameter(*syntax.type_id, mangle_ctx)) {
    return true;
  }
  if(syntax.expression &&
     ast_node_mentions_template_parameter(*syntax.expression, mangle_ctx)) {
    return true;
  }
  return false;
}

static bool template_argument_syntax_contains_pack_expansion(
    const TemplateArgumentSyntax & syntax);

static bool template_id_syntax_contains_pack_expansion(
    const TemplateIdSyntax & syntax)
{
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_contains_pack_expansion(
           syntax.argument_syntaxes[i])) {
      return true;
    }
  }
  return false;
}

static bool template_argument_syntax_contains_pack_expansion(
    const TemplateArgumentSyntax & syntax)
{
  if(syntax.pack_expansion) {
    return true;
  }
  return syntax.template_id &&
         template_id_syntax_contains_pack_expansion(*syntax.template_id);
}

static const semantic_model::AliasTemplateDecl * lookup_alias_template_for_template_id_syntax(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx);

static bool declarator_type_syntax_mentions_template_parameter(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx)
{
  if(node.kind == CppAstKind::identifier) {
    return false;
  }
  if(node.kind == CppAstKind::array_suffix ||
     node.kind == CppAstKind::parameter_clause) {
    return ast_node_mentions_template_parameter_name_only(node, mangle_ctx);
  }
  if(node.kind == CppAstKind::declarator ||
     node.kind == CppAstKind::abstract_declarator ||
     node.kind == CppAstKind::nested_declarator ||
     node.kind == CppAstKind::ptr_operator) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(declarator_type_syntax_mentions_template_parameter(
             node.children[i], mangle_ctx)) {
        return true;
      }
    }
    return false;
  }
  return ast_node_mentions_template_parameter_name_only(node, mangle_ctx);
}

static bool non_type_template_parameter_type_syntax_is_dependent(
    const TemplateParameterInfo & parameter,
    const TypeMangleContext * mangle_ctx)
{
  if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE) {
    return false;
  }
  const bool spec =
      parameter.non_type_decl_specifier_seq &&
      ast_node_mentions_template_parameter_name_only(
          *parameter.non_type_decl_specifier_seq, mangle_ctx);
  const bool decl =
      parameter.non_type_declarator &&
      declarator_type_syntax_mentions_template_parameter(
          *parameter.non_type_declarator, mangle_ctx);
  const bool abstract =
      parameter.non_type_abstract_declarator &&
      declarator_type_syntax_mentions_template_parameter(
          *parameter.non_type_abstract_declarator, mangle_ctx);
  return spec || decl || abstract;
}

static bool non_type_template_parameter_type_mentions_direct_template_parameter(
    const TemplateParameterInfo & parameter,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx ||
     !mangle_ctx->template_parameters ||
     !mangle_ctx->template_parameters->parameters) {
    return false;
  }

  TypeMangleContext direct_ctx = *mangle_ctx;
  direct_ctx.owner_template_parameters = nullptr;
  direct_ctx.owner_template_arguments = nullptr;
  return non_type_template_parameter_type_syntax_is_dependent(parameter,
                                                              &direct_ctx) ||
         type_mentions_function_template_parameter_slice(parameter.value_type,
                                                        mangle_ctx);
}

static string qualified_name_syntax_key_text(const QualifiedName & name)
{
  string out;
  if(name.rooted) {
    out += "::";
  }
  for(size_t i = 0; i < name.qualifiers.size(); ++i) {
    if(!out.empty() &&
       (out.size() < 2 || out.compare(out.size() - 2, 2, "::") != 0)) {
      out += "::";
    }
    out += strip_leading_template_disambiguator(name.qualifiers[i]);
  }
  if(!out.empty() &&
     (out.size() < 2 || out.compare(out.size() - 2, 2, "::") != 0)) {
    out += "::";
  }
  out += strip_leading_template_disambiguator(name.name);
  return out;
}

static string template_id_syntax_key_text(const TemplateIdSyntax & syntax)
{
  string out = qualified_name_syntax_key_text(syntax.name);
  out += '<';
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(i != 0) {
      out += ", ";
    }
    out += trim_space(syntax.argument_syntaxes[i].text);
    if(syntax.argument_syntaxes[i].pack_expansion &&
       out.size() >= 3 &&
       out.compare(out.size() - 3, 3, "...") != 0) {
      out += "...";
    }
  }
  out += '>';
  return out;
}

static string qualified_template_id_syntax_key_text(
    const TemplateIdSyntax & syntax,
    const QualifiedName & qualified)
{
  QualifiedName template_name = syntax.name;
  template_name.rooted = qualified.rooted;
  template_name.qualifiers = qualified.qualifiers;
  template_name.name = strip_leading_template_disambiguator(syntax.name.name);
  string text = qualified_name_syntax_key_text(template_name);
  const string final_text = template_id_syntax_key_text(syntax);
  const size_t last_sep = text.rfind("::");
  return last_sep == string::npos ?
             final_text :
             text.substr(0, last_sep + 2) + final_text;
}

static TemplateArgumentSyntax clone_template_argument_syntax_for_mangling(
    const TemplateArgumentSyntax & source);
static CppAstNode clone_ast_node_for_mangling(const CppAstNode & source);

static TemplateIdSyntax clone_template_id_syntax_for_mangling(
    const TemplateIdSyntax & source)
{
  TemplateIdSyntax out;
  out.name = source.name;
  out.source_location_id = source.source_location_id;
  out.qualifier_template_id_syntaxes.reserve(
      source.qualifier_template_id_syntaxes.size());
  for(size_t i = 0; i < source.qualifier_template_id_syntaxes.size(); ++i) {
    out.qualifier_template_id_syntaxes.push_back(
        clone_template_id_syntax_for_mangling(
            source.qualifier_template_id_syntaxes[i]));
  }
  out.arguments = source.arguments;
  out.argument_syntaxes.reserve(source.argument_syntaxes.size());
  for(size_t i = 0; i < source.argument_syntaxes.size(); ++i) {
    out.argument_syntaxes.push_back(
        clone_template_argument_syntax_for_mangling(source.argument_syntaxes[i]));
  }
  return out;
}

static TemplateArgumentSyntax clone_template_argument_syntax_for_mangling(
    const TemplateArgumentSyntax & source)
{
  TemplateArgumentSyntax out;
  out.text = source.text;
  out.source_text = source.source_text;
  out.pack_expansion = source.pack_expansion;
  out.dependent = source.dependent;
  out.has_source_token_start = source.has_source_token_start;
  out.source_token_start = source.source_token_start;
  out.source_location_id = source.source_location_id;
  out.resolved_type = source.resolved_type;
  if(source.template_id) {
    out.template_id.reset(
        new TemplateIdSyntax(clone_template_id_syntax_for_mangling(*source.template_id)));
  }
  if(source.type_id) {
    out.type_id.reset(new CppAstNode(clone_ast_node_for_mangling(*source.type_id)));
  }
  if(source.source_type_id) {
    out.source_type_id = source.source_type_id;
  }
  if(source.expression) {
    out.expression.reset(new CppAstNode(clone_ast_node_for_mangling(*source.expression)));
  }
  return out;
}

static CppAstNode clone_ast_node_for_mangling(const CppAstNode & source)
{
  CppAstNode out;
  out.kind = source.kind;
  out.value = source.value;
  out.semantic_type = source.semantic_type;
  out.builtin_type_transform_name = source.builtin_type_transform_name;
  out.has_leading_typename = source.has_leading_typename;
  out.has_exception_type_id_syntaxes = source.has_exception_type_id_syntaxes;
  out.linkage_has_braces = source.linkage_has_braces;
  out.has_no_unique_address = source.has_no_unique_address;
  out.has_using_if_exists = source.has_using_if_exists;
  out.has_exclude_from_explicit_instantiation =
      source.has_exclude_from_explicit_instantiation;
  out.asm_label = source.asm_label;
  out.abi_tags = source.abi_tags;
  out.alignment_specifiers = source.alignment_specifiers;
  out.is_final_specifier = source.is_final_specifier;
  out.uses_assignment_form = source.uses_assignment_form;
  out.has_token = source.has_token;
  out.token_kind = source.token_kind;
  out.simple_type = source.simple_type;
  out.token_start = source.token_start;
  out.token_end = source.token_end;
  out.source_location_id = source.source_location_id;
  out.name_lookup_snapshot = source.name_lookup_snapshot;
  if(source.qualified_name_syntax) {
    out.qualified_name_syntax.reset(new QualifiedName(*source.qualified_name_syntax));
  }
  if(source.template_id_syntax) {
    out.template_id_syntax.reset(
        new TemplateIdSyntax(clone_template_id_syntax_for_mangling(
            *source.template_id_syntax)));
  }
  if(source.conversion_type_id_syntax) {
    out.conversion_type_id_syntax.reset(
        new CppAstNode(clone_ast_node_for_mangling(
            *source.conversion_type_id_syntax)));
  }
  if(source.base_type_syntax) {
    out.base_type_syntax.reset(
        new CppAstNode(clone_ast_node_for_mangling(*source.base_type_syntax)));
  }
  out.qualifier_template_id_syntaxes.reserve(
      source.qualifier_template_id_syntaxes.size());
  for(size_t i = 0; i < source.qualifier_template_id_syntaxes.size(); ++i) {
    out.qualifier_template_id_syntaxes.push_back(
        clone_template_id_syntax_for_mangling(
            source.qualifier_template_id_syntaxes[i]));
  }
  out.qualifier_type_syntaxes.reserve(source.qualifier_type_syntaxes.size());
  for(size_t i = 0; i < source.qualifier_type_syntaxes.size(); ++i) {
    out.qualifier_type_syntaxes.push_back(
        clone_ast_node_for_mangling(source.qualifier_type_syntaxes[i]));
  }
  out.exception_type_id_syntaxes.reserve(
      source.exception_type_id_syntaxes.size());
  for(size_t i = 0; i < source.exception_type_id_syntaxes.size(); ++i) {
    out.exception_type_id_syntaxes.push_back(
        clone_ast_node_for_mangling(source.exception_type_id_syntaxes[i]));
  }
  out.alignment_specifier_nodes.reserve(
      source.alignment_specifier_nodes.size());
  for(size_t i = 0; i < source.alignment_specifier_nodes.size(); ++i) {
    out.alignment_specifier_nodes.push_back(
        clone_ast_node_for_mangling(source.alignment_specifier_nodes[i]));
  }
  out.children.reserve(source.children.size());
  for(size_t i = 0; i < source.children.size(); ++i) {
    out.children.push_back(clone_ast_node_for_mangling(source.children[i]));
  }
  return out;
}

static CppAstNode make_semantic_type_id_node_for_mangling(
    const TypePtr & type,
    const string & text)
{
  CppAstNode type_id;
  type_id.kind = CppAstKind::type_id;
  type_id.value = text;
  type_id.semantic_type = type;

  CppAstNode specifiers;
  specifiers.kind = CppAstKind::type_specifier_seq;
  specifiers.value = text;
  specifiers.semantic_type = type;

  CppAstNode type_name;
  type_name.kind = CppAstKind::type_name;
  type_name.value = text;
  type_name.semantic_type = type;

  specifiers.children.push_back(type_name);
  type_id.children.push_back(specifiers);
  return type_id;
}

static TypePtr make_dependent_builtin_type_transform_type_for_mangling(
    const string & builtin_name,
    const string & arg_text,
    const TypePtr & arg_type)
{
  if(builtin_name.empty() || !arg_type) {
    return TypePtr();
  }
  const string display =
      builtin_name + "(" +
      (trim_space(arg_text).empty() ? template_argument_type_text(arg_type) :
                                      trim_space(arg_text)) +
      ")";
  TypePtr result =
      make_semantic_named(display,
                          Type::NSK_DEPENDENT_TYPE,
                          string("$builtin-type-transform:") + builtin_name +
                              "|" + template_argument_type_text(arg_type),
                          true);
  TypePtr base = strip_top_level_cv(result);
  if(base && base->kind == Type::TK_NAMED) {
    base->inner = arg_type;
  }
  return result;
}

static TypePtr template_argument_syntax_semantic_type_for_mangling(
    const TemplateArgumentSyntax & syntax)
{
  if(syntax.resolved_type) {
    return syntax.resolved_type;
  }
  if(!syntax.type_id) {
    return TypePtr();
  }
  if(syntax.type_id->semantic_type) {
    return syntax.type_id->semantic_type;
  }
  if(!syntax.type_id->children.empty()) {
    const CppAstNode & specifiers = syntax.type_id->children[0];
    if(specifiers.semantic_type) {
      return specifiers.semantic_type;
    }
    if(!specifiers.children.empty() &&
       specifiers.children[0].semantic_type) {
      return specifiers.children[0].semantic_type;
    }
  }
  return TypePtr();
}

static bool attach_dependent_builtin_type_transform_argument_for_mangling(
    CppAstNode & node,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgumentSyntax> & arguments)
{
  const string & builtin_name = node.builtin_type_transform_name;
  if(!node.base_type_syntax ||
     !is_mangleable_builtin_type_transform_name(builtin_name)) {
    return false;
  }

  const size_t count = std::min(parameters.size(), arguments.size());
  for(size_t i = 0; i < count; ++i) {
    if(!type_id_ast_is_template_type_parameter(*node.base_type_syntax,
                                               parameters[i])) {
      continue;
    }
    TypePtr arg_type =
        template_argument_syntax_semantic_type_for_mangling(arguments[i]);
    if(arg_type && type_has_dependent_mangle_state(arg_type)) {
      TypePtr transformed =
          make_dependent_builtin_type_transform_type_for_mangling(
              builtin_name,
              arguments[i].text,
              arg_type);
      if(!transformed) {
        return false;
      }
      node.value = builtin_name + "(" + trim_space(arguments[i].text) + ")";
      node.builtin_type_transform_name = builtin_name;
      node.semantic_type = transformed;
      return true;
    }
    if(arguments[i].type_id) {
      node.value = builtin_name + "(" + trim_space(arguments[i].text) + ")";
      node.builtin_type_transform_name = builtin_name;
      node.children.clear();
      node.children.push_back(clone_ast_node_for_mangling(*arguments[i].type_id));
      return true;
    }
    return false;
  }
  return false;
}

static const vector<TemplateParameterInfo> * lookup_template_parameters_for_template_id_syntax(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx || !mangle_ctx->lookup_scope) {
    return nullptr;
  }

  const string base_name =
      strip_leading_template_disambiguator(syntax.name.name);
  if(base_name.empty()) {
    return nullptr;
  }

  if(syntax.name.qualifiers.empty()) {
    for(const semantic_model::Scope * scope = mangle_ctx->lookup_scope;
        scope;
        scope = scope->parent) {
      if(const semantic_model::ClassTemplateDecl * class_found =
             find_unqualified_class_template_in_mangle_scope(*scope, base_name)) {
        return &class_found->parameters;
      }
      if(const semantic_model::AliasTemplateDecl * alias_found =
             find_unqualified_alias_template_in_mangle_scope(*scope, base_name)) {
        return &alias_found->parameters;
      }
    }
    if(const semantic_model::ClassTemplateDecl * class_found =
           lookup_class_template_for_template_id_syntax(syntax, mangle_ctx)) {
      return &class_found->parameters;
    }
    return nullptr;
  }

  const semantic_model::Scope * scope = root_scope(mangle_ctx->lookup_scope);
  for(size_t i = 0; scope && i < syntax.name.qualifiers.size(); ++i) {
    const string qualifier =
        semantic_utils::strip_trailing_top_level_template_arguments(
            trim_space(syntax.name.qualifiers[i]));
    map<string, semantic_model::Scope *>::const_iterator found =
        scope->namespace_bindings.find(qualifier);
    scope = found == scope->namespace_bindings.end() ? nullptr : found->second;
  }
  if(!scope) {
    return nullptr;
  }
  if(const semantic_model::ClassTemplateDecl * class_found =
         find_unqualified_class_template_in_mangle_scope(*scope, base_name)) {
    return &class_found->parameters;
  }
  if(const semantic_model::AliasTemplateDecl * alias_found =
         find_unqualified_alias_template_in_mangle_scope(*scope, base_name)) {
    return &alias_found->parameters;
  }
  return nullptr;
}

static const semantic_model::AliasTemplateDecl * lookup_alias_template_for_template_id_syntax(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx || !mangle_ctx->lookup_scope) {
    return nullptr;
  }

  const string base_name =
      strip_leading_template_disambiguator(syntax.name.name);
  if(base_name.empty()) {
    return nullptr;
  }

  if(syntax.name.qualifiers.empty()) {
    for(const semantic_model::Scope * scope = mangle_ctx->lookup_scope;
        scope;
        scope = scope->parent) {
      if(const semantic_model::AliasTemplateDecl * found =
             find_unqualified_alias_template_in_mangle_scope(*scope, base_name)) {
        return found;
      }
    }
    if(!mangle_ctx->lexical_scope_syntax.name.empty()) {
      const QualifiedName & lexical = mangle_ctx->lexical_scope_syntax;
      if(!lexical.rooted) {
        const semantic_model::Scope * scope = root_scope(mangle_ctx->lookup_scope);
        vector<string> parts = lexical.qualifiers;
        parts.push_back(lexical.name);
        size_t part_index = 0;
        if(scope &&
           scope->namespace_scope &&
           !scope->name.empty() &&
           !parts.empty() &&
           scope->name == parts[0]) {
          part_index = 1;
        }
        for(size_t i = part_index; scope && i < parts.size(); ++i) {
          map<string, semantic_model::Scope *>::const_iterator ns_found =
              scope->namespace_bindings.find(parts[i]);
          scope = ns_found == scope->namespace_bindings.end() ? nullptr : ns_found->second;
        }
        if(scope) {
          if(const semantic_model::AliasTemplateDecl * found =
                 find_unqualified_alias_template_in_mangle_scope(*scope, base_name)) {
            return found;
          }
        }
      }
    }
    return nullptr;
  }

  const semantic_model::Scope * scope = root_scope(mangle_ctx->lookup_scope);
  for(size_t i = 0; scope && i < syntax.name.qualifiers.size(); ++i) {
    const string qualifier =
        semantic_utils::strip_trailing_top_level_template_arguments(
            trim_space(syntax.name.qualifiers[i]));
    map<string, semantic_model::Scope *>::const_iterator found =
        scope->namespace_bindings.find(qualifier);
    scope = found == scope->namespace_bindings.end() ? nullptr : found->second;
  }
  if(!scope) {
    return nullptr;
  }
  return find_unqualified_alias_template_in_mangle_scope(*scope, base_name);
}

static const semantic_model::Scope * template_id_default_argument_scope_for_mangling(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(const semantic_model::ClassTemplateDecl * class_template =
         lookup_class_template_for_template_id_syntax(syntax, mangle_ctx)) {
    return class_template->declaring_scope;
  }
  if(const semantic_model::AliasTemplateDecl * alias_template =
         lookup_alias_template_for_template_id_syntax(syntax, mangle_ctx)) {
    return alias_template->declaring_scope;
  }
  return nullptr;
}

static bool template_argument_syntax_has_dependent_alias_template_id(
    const TemplateArgumentSyntax & syntax,
    const TypeMangleContext * mangle_ctx);
static bool template_argument_syntax_has_dependent_decltype_alias_template_id(
    const TemplateArgumentSyntax & syntax,
    const TypeMangleContext * mangle_ctx);

static bool template_id_syntax_has_dependent_alias_template_argument(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_has_dependent_alias_template_id(
           syntax.argument_syntaxes[i], mangle_ctx)) {
      return true;
    }
  }
  return false;
}

static bool template_argument_syntax_has_dependent_alias_template_id(
    const TemplateArgumentSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(!template_argument_syntax_mentions_template_parameter(syntax, mangle_ctx)) {
    return false;
  }
  if(syntax.template_id) {
    if(lookup_alias_template_for_template_id_syntax(*syntax.template_id, mangle_ctx)) {
      return true;
    }
    if(template_id_syntax_has_dependent_alias_template_argument(
           *syntax.template_id, mangle_ctx)) {
      return true;
    }
  }
  return false;
}

static bool template_id_syntax_has_dependent_decltype_alias_template_argument(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_has_dependent_decltype_alias_template_id(
           syntax.argument_syntaxes[i], mangle_ctx)) {
      return true;
    }
  }
  return false;
}

static bool ast_contains_decltype_specifier(const CppAstNode & node)
{
  if(node.kind == CppAstKind::decltype_specifier) {
    return true;
  }
  if(node.conversion_type_id_syntax &&
     ast_contains_decltype_specifier(*node.conversion_type_id_syntax)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(ast_contains_decltype_specifier(node.qualifier_type_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    if(ast_contains_decltype_specifier(node.exception_type_id_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_contains_decltype_specifier(node.children[i])) {
      return true;
    }
  }
  return false;
}

static bool alias_template_expansion_contains_decltype(
    const semantic_model::AliasTemplateDecl * alias_template)
{
  return alias_template &&
         alias_template->type_id &&
         ast_contains_decltype_specifier(*alias_template->type_id);
}

static bool ast_node_has_dependent_decltype_alias_template_id(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx);

static bool template_id_syntax_is_dependent_decltype_alias_template_id(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(!template_id_syntax_mentions_template_parameter(syntax, mangle_ctx)) {
    return false;
  }
  if(alias_template_expansion_contains_decltype(
         lookup_alias_template_for_template_id_syntax(syntax, mangle_ctx))) {
    return true;
  }
  return template_id_syntax_has_dependent_decltype_alias_template_argument(
      syntax, mangle_ctx);
}

static bool template_argument_syntax_has_dependent_decltype_alias_template_id(
    const TemplateArgumentSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(!template_argument_syntax_mentions_template_parameter(syntax, mangle_ctx)) {
    return false;
  }
  if(syntax.template_id &&
     template_id_syntax_is_dependent_decltype_alias_template_id(
         *syntax.template_id, mangle_ctx)) {
    return true;
  }
  if(syntax.type_id &&
     ast_node_has_dependent_decltype_alias_template_id(*syntax.type_id, mangle_ctx)) {
    return true;
  }
  if(syntax.expression &&
     ast_node_has_dependent_decltype_alias_template_id(*syntax.expression, mangle_ctx)) {
    return true;
  }
  return false;
}

static bool ast_node_has_dependent_decltype_alias_template_id(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx)
{
  if(node.template_id_syntax &&
     template_id_syntax_is_dependent_decltype_alias_template_id(
         *node.template_id_syntax, mangle_ctx)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_is_dependent_decltype_alias_template_id(
           node.qualifier_template_id_syntaxes[i], mangle_ctx)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_has_dependent_decltype_alias_template_id(node.children[i], mangle_ctx)) {
      return true;
    }
  }
  return false;
}

static bool type_ast_references_alias_template(const CppAstNode & node,
                                               const TypeMangleContext * mangle_ctx)
{
  if(node.template_id_syntax &&
     lookup_alias_template_for_template_id_syntax(*node.template_id_syntax,
                                                  mangle_ctx)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(lookup_alias_template_for_template_id_syntax(
           node.qualifier_template_id_syntaxes[i], mangle_ctx)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(type_ast_references_alias_template(node.children[i], mangle_ctx)) {
      return true;
    }
  }
  return false;
}

static TypePtr template_parameter_type_for_mangling(
    const TemplateParameterInfo & parameter)
{
  if(parameter.kind != TemplateParameterInfo::TP_TYPE ||
     parameter.name.empty()) {
    return TypePtr();
  }

  const string key =
      parameter.placeholder_key.empty() ?
          string("template-parameter ") + parameter.name :
          parameter.placeholder_key;
  return make_named(parameter.name, key, true);
}

static bool template_argument_syntax_is_parameter_name(
    const TemplateArgumentSyntax & syntax,
    const TemplateParameterInfo & parameter)
{
  string stripped = trim_elaborated_type_prefix(syntax.text);
  if(stripped.size() >= 3 &&
     stripped.compare(stripped.size() - 3, 3, "...") == 0) {
    stripped = trim_space(stripped.substr(0, stripped.size() - 3));
  }
  if(template_parameter_identifier_matches(parameter, stripped)) {
    return true;
  }

  if(syntax.type_id &&
     syntax.type_id->children.size() == 1) {
    const CppAstNode & specifiers = syntax.type_id->children[0];
    if((specifiers.kind == CppAstKind::type_specifier_seq ||
        specifiers.kind == CppAstKind::decl_specifier_seq) &&
       specifiers.children.size() == 1) {
      const string value =
          trim_elaborated_type_prefix(specifiers.children[0].value);
      if(template_parameter_identifier_matches(parameter, value)) {
        return true;
      }
    }
  }
  if(syntax.expression &&
     template_parameter_identifier_matches(
         parameter, trim_space(syntax.expression->value))) {
    return true;
  }
  return false;
}

static string alias_template_argument_replacement_text(
    const TemplateArgumentSyntax & argument)
{
  if(!argument.text.empty()) {
    return argument.text;
  }
  if(argument.type_id && !argument.type_id->value.empty()) {
    return argument.type_id->value;
  }
  if(argument.expression && !argument.expression->value.empty()) {
    return argument.expression->value;
  }
  if(argument.template_id) {
    return template_id_syntax_key_text(*argument.template_id);
  }
  return string();
}

static string alias_template_argument_replacement_text(
    const DependentAliasTemplateArgumentSyntax & argument)
{
  if(!argument.syntax.text.empty()) {
    return argument.syntax.text;
  }
  if(!argument.text.empty()) {
    return argument.text;
  }
  if(argument.type) {
    return template_argument_type_text(argument.type);
  }
  return string();
}

static TemplateArgumentSyntax alias_template_argument_replacement_syntax(
    const TemplateArgumentSyntax & argument)
{
  return clone_template_argument_syntax_for_mangling(argument);
}

static TemplateArgumentSyntax alias_template_argument_replacement_syntax(
    const DependentAliasTemplateArgumentSyntax & argument);

static bool alias_parameter_text_matches(const string & text,
                                         const TemplateParameterInfo & parameter)
{
  return template_parameter_identifier_matches(parameter, text);
}

static bool replacement_syntax_qualified_name(
    const TemplateArgumentSyntax & replacement,
    QualifiedName & out,
    TemplateIdSyntax * template_id)
{
  if(template_id) {
    *template_id = TemplateIdSyntax();
  }
  const string replacement_text =
      trim_space(alias_template_argument_replacement_text(replacement));
  if(replacement.template_id && !replacement.template_id->name.name.empty()) {
    TemplateIdSyntax cloned =
        clone_template_id_syntax_for_mangling(*replacement.template_id);
    out = cloned.name;
    if(template_id) {
      cloned.name.qualifiers.clear();
      *template_id = cloned;
    }
    return !out.name.empty();
  }
  const function<const QualifiedName *(const CppAstNode &)> find_qualified =
      [&](const CppAstNode & node) -> const QualifiedName *
      {
        if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
          if(!qualified->name.empty()) {
            return qualified;
          }
        }
        for(size_t i = 0; i < node.children.size(); ++i) {
          if(const QualifiedName * qualified = find_qualified(node.children[i])) {
            return qualified;
          }
        }
        return nullptr;
      };
  const QualifiedName * qualified = nullptr;
  if(replacement.type_id) {
    qualified = find_qualified(*replacement.type_id);
  }
  if(!qualified && replacement.source_type_id) {
    qualified = find_qualified(*replacement.source_type_id);
  }
  if(!qualified && replacement.expression) {
    qualified = find_qualified(*replacement.expression);
  }
  if(qualified) {
    out = *qualified;
    return true;
  }
  TypePtr resolved = strip_top_level_cv(replacement.resolved_type);
  if(resolved &&
     resolved->kind == Type::TK_NAMED &&
     !resolved->named_qualified_name_syntax.name.empty()) {
    out = resolved->named_qualified_name_syntax;
    return true;
  }
  if(replacement_text.empty() || replacement_text.find("::") != string::npos) {
    return false;
  }
  out = QualifiedName();
  out.name = replacement_text;
  return true;
}

template <typename ArgumentSyntax>
static bool substitute_alias_template_arguments_in_qualified_name(
    CppAstNode & node,
    const vector<TemplateParameterInfo> & parameters,
    const vector<ArgumentSyntax> & arguments)
{
  if(!node.qualified_name_syntax) {
    return false;
  }
  QualifiedName qualified = *node.qualified_name_syntax;
  if(qualified.qualifiers.empty()) {
    return false;
  }
  const size_t count = std::min(parameters.size(), arguments.size());
  for(size_t q = 0; q < qualified.qualifiers.size(); ++q) {
    for(size_t i = 0; i < count; ++i) {
      if(!alias_parameter_text_matches(qualified.qualifiers[q], parameters[i])) {
        continue;
      }
      const TemplateArgumentSyntax replacement =
          alias_template_argument_replacement_syntax(arguments[i]);
      QualifiedName replacement_name;
      TemplateIdSyntax replacement_template_id;
      if(!replacement_syntax_qualified_name(
             replacement, replacement_name, &replacement_template_id)) {
        return false;
      }

      vector<string> new_qualifiers;
      new_qualifiers.reserve(qualified.qualifiers.size() +
                             replacement_name.qualifiers.size());
      new_qualifiers.insert(new_qualifiers.end(),
                            qualified.qualifiers.begin(),
                            qualified.qualifiers.begin() + q);
      new_qualifiers.insert(new_qualifiers.end(),
                            replacement_name.qualifiers.begin(),
                            replacement_name.qualifiers.end());
      const size_t replacement_component_index = new_qualifiers.size();
      new_qualifiers.push_back(replacement_name.name);
      new_qualifiers.insert(new_qualifiers.end(),
                            qualified.qualifiers.begin() + q + 1,
                            qualified.qualifiers.end());

      vector<TemplateIdSyntax> new_qualifier_template_ids(
          new_qualifiers.size());
      for(size_t old_i = 0; old_i < q &&
             old_i < node.qualifier_template_id_syntaxes.size(); ++old_i) {
        new_qualifier_template_ids[old_i] =
            clone_template_id_syntax_for_mangling(
                node.qualifier_template_id_syntaxes[old_i]);
      }
      if(!replacement_template_id.name.name.empty()) {
        new_qualifier_template_ids[replacement_component_index] =
            replacement_template_id;
      }
      const size_t shifted = replacement_name.qualifiers.size();
      for(size_t old_i = q + 1; old_i < qualified.qualifiers.size() &&
             old_i < node.qualifier_template_id_syntaxes.size(); ++old_i) {
        new_qualifier_template_ids[old_i + shifted] =
            clone_template_id_syntax_for_mangling(
                node.qualifier_template_id_syntaxes[old_i]);
      }

      qualified.qualifiers = new_qualifiers;
      node.qualified_name_syntax.reset(new QualifiedName(qualified));
      node.qualifier_template_id_syntaxes = new_qualifier_template_ids;
      node.qualifier_type_syntaxes.clear();
      node.value = qualified_name_syntax_key_text(qualified);
      return true;
    }
  }
  return false;
}

template <typename ArgumentSyntax>
static bool substitute_alias_template_arguments_in_text(
    string & text,
    const vector<TemplateParameterInfo> & parameters,
    const vector<ArgumentSyntax> & arguments)
{
  const size_t count = std::min(parameters.size(), arguments.size());
  for(size_t i = 0; i < count; ++i) {
    if(!alias_parameter_text_matches(text, parameters[i])) {
      continue;
    }
    const string replacement =
        trim_space(alias_template_argument_replacement_text(arguments[i]));
    if(replacement.empty()) {
      return false;
    }
    text = replacement;
    return true;
  }

  return false;
}

static bool substitute_alias_template_arguments_in_node(
    CppAstNode & node,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgumentSyntax> & arguments);
static bool substitute_alias_template_type_id_node(
    CppAstNode & node,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgumentSyntax> & arguments);

static bool substitute_alias_template_arguments_in_syntax(
    TemplateArgumentSyntax & syntax,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgumentSyntax> & arguments)
{
  bool changed = false;
  const size_t count = std::min(parameters.size(), arguments.size());
  for(size_t i = 0; i < count; ++i) {
    if(template_argument_syntax_is_parameter_name(syntax, parameters[i])) {
      syntax = clone_template_argument_syntax_for_mangling(arguments[i]);
      return true;
    }
  }
  changed =
      substitute_alias_template_arguments_in_text(
          syntax.text, parameters, arguments) ||
      changed;
  if(syntax.template_id) {
    for(size_t i = 0; i < syntax.template_id->argument_syntaxes.size(); ++i) {
      changed =
          substitute_alias_template_arguments_in_syntax(
              syntax.template_id->argument_syntaxes[i],
              parameters,
              arguments) ||
          changed;
    }
  }
  if(syntax.type_id) {
    changed =
        substitute_alias_template_arguments_in_node(*syntax.type_id, parameters, arguments) ||
        changed;
  }
  if(syntax.expression) {
    changed =
        substitute_alias_template_arguments_in_node(*syntax.expression, parameters, arguments) ||
        changed;
  }
  return changed;
}

static bool substitute_alias_template_arguments_in_template_id(
    TemplateIdSyntax & syntax,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgumentSyntax> & arguments)
{
  bool changed = false;
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    const bool argument_changed =
        substitute_alias_template_arguments_in_syntax(
            syntax.argument_syntaxes[i],
            parameters,
            arguments);
    changed = argument_changed || changed;
    if(i < syntax.arguments.size()) {
      syntax.arguments[i] = syntax.argument_syntaxes[i].text;
    }
  }
  return changed;
}

static bool substitute_alias_template_arguments_in_node(
    CppAstNode & node,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgumentSyntax> & arguments)
{
  bool changed = false;
  if(substitute_alias_template_type_id_node(node, parameters, arguments)) {
    return true;
  }
  const bool attached_semantic =
      attach_dependent_builtin_type_transform_argument_for_mangling(
          node,
          parameters,
          arguments);
  changed = attached_semantic || changed;
  changed =
      substitute_alias_template_arguments_in_qualified_name(
          node, parameters, arguments) ||
      changed;
  if(substitute_alias_template_arguments_in_text(
         node.value, parameters, arguments)) {
    changed = true;
  }
  if(node.template_id_syntax) {
    changed =
        substitute_alias_template_arguments_in_template_id(
            *node.template_id_syntax,
            parameters,
            arguments) ||
        changed;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    changed =
        substitute_alias_template_arguments_in_template_id(
            node.qualifier_template_id_syntaxes[i],
            parameters,
            arguments) ||
        changed;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    changed =
        substitute_alias_template_arguments_in_node(
            node.children[i],
            parameters,
            arguments) ||
        changed;
  }
  if(changed && !attached_semantic) {
    node.semantic_type.reset();
  }
  return changed;
}

static TemplateArgumentSyntax dependent_alias_argument_syntax_for_mangling(
    const DependentAliasTemplateArgumentSyntax & argument)
{
  TemplateArgumentSyntax syntax =
      clone_template_argument_syntax_for_mangling(argument.syntax);
  if(syntax.text.empty()) {
    syntax.text = argument.text;
  }
  if(argument.type) {
    syntax.resolved_type = argument.type;
    if(!syntax.type_id) {
      syntax.type_id.reset(new CppAstNode(
          make_semantic_type_id_node_for_mangling(argument.type, syntax.text)));
    }
    if(!type_has_dependent_mangle_state(argument.type)) {
      syntax.pack_expansion = false;
    }
  }
  return syntax;
}

static TemplateArgumentSyntax alias_template_argument_replacement_syntax(
    const DependentAliasTemplateArgumentSyntax & argument)
{
  return dependent_alias_argument_syntax_for_mangling(argument);
}

static bool type_specifier_seq_is_template_type_parameter(
    const CppAstNode & specifiers,
    const TemplateParameterInfo & parameter)
{
  if(parameter.kind != TemplateParameterInfo::TP_TYPE) {
    return false;
  }
  if(specifiers.kind != CppAstKind::type_specifier_seq &&
     specifiers.kind != CppAstKind::decl_specifier_seq) {
    return false;
  }

  const CppAstNode * type_name = nullptr;
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    const CppAstNode & child = specifiers.children[i];
    if(child.value == "const" ||
       child.value == "volatile" ||
       child.value == "typename" ||
       child.value == "class" ||
       child.value == "struct") {
      continue;
    }
    if(type_name) {
      return false;
    }
    type_name = &child;
  }

  return type_name &&
         template_parameter_identifier_matches(parameter, type_name->value);
}

static bool type_id_ast_is_template_type_parameter(
    const CppAstNode & node,
    const TemplateParameterInfo & parameter)
{
  if(node.kind != CppAstKind::type_id ||
     node.children.size() != 1) {
    return false;
  }

  return type_specifier_seq_is_template_type_parameter(node.children[0],
                                                       parameter);
}

static bool make_type_id_node_from_template_argument_syntax_for_mangling(
    const TemplateArgumentSyntax & syntax,
    CppAstNode & out)
{
  if(syntax.type_id) {
    out = clone_ast_node_for_mangling(*syntax.type_id);
    return true;
  }

  const TypePtr semantic_type = syntax.resolved_type;
  const string text = trim_space(syntax.text);
  if(text.empty()) {
    return false;
  }

  out = make_semantic_type_id_node_for_mangling(semantic_type, text);
  if(out.children.empty() || out.children[0].children.empty()) {
    return static_cast<bool>(semantic_type);
  }

  CppAstNode & type_name = out.children[0].children[0];
  if(syntax.template_id) {
    type_name.qualified_name_syntax.reset(
        new QualifiedName(syntax.template_id->name));
    type_name.template_id_syntax.reset(
        new TemplateIdSyntax(
            clone_template_id_syntax_for_mangling(*syntax.template_id)));
  }
  if(semantic_type) {
    out.semantic_type = semantic_type;
    out.children[0].semantic_type = semantic_type;
    type_name.semantic_type = semantic_type;
  }
  return syntax.template_id || semantic_type;
}

static bool replace_type_id_base_specifier_from_template_argument_syntax(
    CppAstNode & node,
    const TemplateArgumentSyntax & replacement)
{
  if(node.kind != CppAstKind::type_id ||
     node.children.empty() ||
     (node.children[0].kind != CppAstKind::type_specifier_seq &&
      node.children[0].kind != CppAstKind::decl_specifier_seq)) {
    return false;
  }

  CppAstNode replacement_type_id;
  if(!make_type_id_node_from_template_argument_syntax_for_mangling(
         replacement,
         replacement_type_id) ||
     replacement_type_id.children.size() != 1 ||
     (replacement_type_id.children[0].kind != CppAstKind::type_specifier_seq &&
      replacement_type_id.children[0].kind != CppAstKind::decl_specifier_seq)) {
    return false;
  }

  CppAstNode replacement_specifiers = replacement_type_id.children[0];
  bool replacement_has_const = false;
  bool replacement_has_volatile = false;
  for(size_t i = 0; i < replacement_specifiers.children.size(); ++i) {
    replacement_has_const =
        replacement_has_const ||
        replacement_specifiers.children[i].value == "const";
    replacement_has_volatile =
        replacement_has_volatile ||
        replacement_specifiers.children[i].value == "volatile";
  }
  for(size_t i = 0; i < node.children[0].children.size(); ++i) {
    const CppAstNode & child = node.children[0].children[i];
    if(child.value == "const") {
      if(!replacement_has_const) {
        replacement_specifiers.children.push_back(child);
        replacement_has_const = true;
      }
    } else if(child.value == "volatile") {
      if(!replacement_has_volatile) {
        replacement_specifiers.children.push_back(child);
        replacement_has_volatile = true;
      }
    }
  }
  node.children[0] = replacement_specifiers;
  node.semantic_type.reset();
  return true;
}

static bool substitute_alias_template_type_id_node(
    CppAstNode & node,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgumentSyntax> & arguments)
{
  const size_t count = std::min(parameters.size(), arguments.size());
  for(size_t i = 0; i < count; ++i) {
    if(!type_id_ast_is_template_type_parameter(node, parameters[i])) {
      continue;
    }
    CppAstNode replacement_type_id;
    if(!make_type_id_node_from_template_argument_syntax_for_mangling(
           arguments[i], replacement_type_id)) {
      return false;
    }
    node = replacement_type_id;
    return true;
  }
  if(node.kind == CppAstKind::type_id &&
     !node.children.empty()) {
    for(size_t i = 0; i < count; ++i) {
      if(!type_specifier_seq_is_template_type_parameter(node.children[0],
                                                        parameters[i])) {
        continue;
      }
      return replace_type_id_base_specifier_from_template_argument_syntax(
          node,
          arguments[i]);
    }
  }
  return false;
}

static bool substitute_dependent_alias_template_type_id_node(
    CppAstNode & node,
    const vector<TemplateParameterInfo> & parameters,
    const vector<DependentAliasTemplateArgumentSyntax> & arguments)
{
  const size_t count = std::min(parameters.size(), arguments.size());
  for(size_t i = 0; i < count; ++i) {
    if(!type_id_ast_is_template_type_parameter(node, parameters[i])) {
      continue;
    }
    const TemplateArgumentSyntax replacement =
        dependent_alias_argument_syntax_for_mangling(arguments[i]);
    CppAstNode replacement_type_id;
    if(!make_type_id_node_from_template_argument_syntax_for_mangling(
           replacement, replacement_type_id)) {
      return false;
    }
    node = replacement_type_id;
    return true;
  }
  if(node.kind == CppAstKind::type_id &&
     !node.children.empty()) {
    for(size_t i = 0; i < count; ++i) {
      if(!type_specifier_seq_is_template_type_parameter(node.children[0],
                                                        parameters[i])) {
        continue;
      }
      const TemplateArgumentSyntax replacement =
          dependent_alias_argument_syntax_for_mangling(arguments[i]);
      return replace_type_id_base_specifier_from_template_argument_syntax(
          node,
          replacement);
    }
  }
  return false;
}

static bool substitute_dependent_alias_template_arguments_in_node(
    CppAstNode & node,
    const vector<TemplateParameterInfo> & parameters,
    const vector<DependentAliasTemplateArgumentSyntax> & arguments);

static bool substitute_dependent_alias_template_arguments_in_syntax(
    TemplateArgumentSyntax & syntax,
    const vector<TemplateParameterInfo> & parameters,
    const vector<DependentAliasTemplateArgumentSyntax> & arguments)
{
  bool changed = false;
  const size_t count = std::min(parameters.size(), arguments.size());
  for(size_t i = 0; i < count; ++i) {
    if(template_argument_syntax_is_parameter_name(syntax, parameters[i])) {
      syntax = dependent_alias_argument_syntax_for_mangling(arguments[i]);
      return true;
    }
  }
  changed =
      substitute_alias_template_arguments_in_text(
          syntax.text, parameters, arguments) ||
      changed;
  if(syntax.template_id) {
    for(size_t i = 0; i < syntax.template_id->argument_syntaxes.size(); ++i) {
      changed =
          substitute_dependent_alias_template_arguments_in_syntax(
              syntax.template_id->argument_syntaxes[i],
              parameters,
              arguments) ||
          changed;
    }
  }
  if(syntax.type_id) {
    changed =
        substitute_dependent_alias_template_arguments_in_node(
            *syntax.type_id,
            parameters,
            arguments) ||
        changed;
  }
  if(syntax.expression) {
    changed =
        substitute_dependent_alias_template_arguments_in_node(
            *syntax.expression,
            parameters,
            arguments) ||
        changed;
  }
  return changed;
}

static bool substitute_dependent_alias_template_arguments_in_template_id(
    TemplateIdSyntax & syntax,
    const vector<TemplateParameterInfo> & parameters,
    const vector<DependentAliasTemplateArgumentSyntax> & arguments)
{
  bool changed = false;
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    const bool argument_changed =
        substitute_dependent_alias_template_arguments_in_syntax(
            syntax.argument_syntaxes[i],
            parameters,
            arguments);
    changed = argument_changed || changed;
    if(i < syntax.arguments.size()) {
      syntax.arguments[i] = syntax.argument_syntaxes[i].text;
    }
  }
  return changed;
}

static bool substitute_dependent_alias_template_arguments_in_node(
    CppAstNode & node,
    const vector<TemplateParameterInfo> & parameters,
    const vector<DependentAliasTemplateArgumentSyntax> & arguments)
{
  bool changed = false;
  bool attached_semantic = false;
  if(substitute_dependent_alias_template_type_id_node(node,
                                                     parameters,
                                                     arguments)) {
    return true;
  }
  const string & builtin_name = node.builtin_type_transform_name;
  if(node.base_type_syntax &&
     is_mangleable_builtin_type_transform_name(builtin_name)) {
    const size_t count = std::min(parameters.size(), arguments.size());
    for(size_t i = 0; i < count; ++i) {
      if(!type_id_ast_is_template_type_parameter(*node.base_type_syntax,
                                                 parameters[i]) ||
         ((!arguments[i].type ||
           !type_has_dependent_mangle_state(arguments[i].type)) &&
          !arguments[i].syntax.type_id)) {
        continue;
      }
      if(arguments[i].type &&
         type_has_dependent_mangle_state(arguments[i].type)) {
        TypePtr transformed =
            make_dependent_builtin_type_transform_type_for_mangling(
                builtin_name,
                arguments[i].text,
                arguments[i].type);
        if(transformed) {
          node.value =
              builtin_name + "(" + trim_space(arguments[i].text) + ")";
          node.builtin_type_transform_name = builtin_name;
          node.semantic_type = transformed;
          attached_semantic = true;
          changed = true;
        }
      } else if(arguments[i].syntax.type_id) {
        node.value =
            builtin_name + "(" + trim_space(arguments[i].text) + ")";
        node.builtin_type_transform_name = builtin_name;
        node.children.clear();
        node.children.push_back(
            clone_ast_node_for_mangling(*arguments[i].syntax.type_id));
        changed = true;
      }
      break;
    }
  }
  if(substitute_alias_template_arguments_in_text(
         node.value, parameters, arguments)) {
    changed = true;
  }
  changed =
      substitute_alias_template_arguments_in_qualified_name(
          node, parameters, arguments) ||
      changed;
  if(node.template_id_syntax) {
    changed =
        substitute_dependent_alias_template_arguments_in_template_id(
            *node.template_id_syntax,
            parameters,
            arguments) ||
        changed;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    changed =
        substitute_dependent_alias_template_arguments_in_template_id(
            node.qualifier_template_id_syntaxes[i],
            parameters,
            arguments) ||
        changed;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    changed =
        substitute_dependent_alias_template_arguments_in_node(
            node.children[i],
            parameters,
            arguments) ||
        changed;
  }
  if(changed && !attached_semantic) {
    node.semantic_type.reset();
  }
  return changed;
}

static string ast_leaf_text_for_mangling(const CppAstNode & node)
{
  string text = trim_space(node_text(node));
  if(!text.empty()) {
    return text;
  }
  text = trim_space(node.value);
  for(size_t i = 0; i < node.children.size(); ++i) {
    const string part = ast_leaf_text_for_mangling(node.children[i]);
    if(part.empty()) {
      continue;
    }
    if(!text.empty()) {
      text += " ";
    }
    text += part;
  }
  return trim_space(text);
}

static string default_template_argument_text_for_mangling(
    const TemplateParameterInfo & parameter,
    const CppAstNode & node)
{
  if(parameter.kind == TemplateParameterInfo::TP_TYPE &&
     node.kind == CppAstKind::type_id) {
    return ast_leaf_text_for_mangling(node);
  }
  return ast_leaf_text_for_mangling(node);
}

static void qualify_default_template_argument_node_for_mangling(
    CppAstNode & node,
    const semantic_model::Scope * default_argument_scope)
{
  if(!default_argument_scope) {
    return;
  }

  TypeMangleContext default_ctx;
  default_ctx.lookup_scope = default_argument_scope;
  scope_prefix_text_for_template_decl(default_argument_scope,
                                      default_ctx.lexical_scope);
  const auto qualify_template_id =
      [&default_ctx](TemplateIdSyntax & syntax) -> void
      {
        QualifiedName qualified;
        if(qualify_template_id_syntax_from_lookup(syntax,
                                                  &default_ctx,
                                                  qualified)) {
          syntax.name = qualified;
        }
      };
  const function<void(TemplateArgumentSyntax &)> qualify_argument =
      [&](TemplateArgumentSyntax & syntax) -> void
      {
        if(syntax.template_id) {
          qualify_template_id(*syntax.template_id);
          for(size_t i = 0; i < syntax.template_id->argument_syntaxes.size(); ++i) {
            qualify_argument(syntax.template_id->argument_syntaxes[i]);
          }
        }
        if(syntax.type_id) {
          qualify_default_template_argument_node_for_mangling(
              *syntax.type_id,
              default_argument_scope);
        }
        if(syntax.expression) {
          qualify_default_template_argument_node_for_mangling(
              *syntax.expression,
              default_argument_scope);
        }
      };
  const function<void(TemplateIdSyntax &)> qualify_template_id_tree =
      [&](TemplateIdSyntax & syntax) -> void
      {
        qualify_template_id(syntax);
        for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
          qualify_argument(syntax.argument_syntaxes[i]);
        }
      };

  if(node.template_id_syntax) {
    qualify_template_id_tree(*node.template_id_syntax);
    if(node.qualified_name_syntax &&
       !node.template_id_syntax->name.name.empty()) {
      node.qualified_name_syntax->rooted = node.template_id_syntax->name.rooted;
      node.qualified_name_syntax->qualifiers =
          node.template_id_syntax->name.qualifiers;
      node.qualified_name_syntax->name = node.template_id_syntax->name.name;
    }
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    qualify_template_id_tree(node.qualifier_template_id_syntaxes[i]);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    qualify_default_template_argument_node_for_mangling(
        node.children[i],
        default_argument_scope);
  }
}

static void clear_default_type_argument_semantics_for_mangling(CppAstNode & node);

static void clear_default_type_argument_semantics_for_mangling(
    TemplateArgumentSyntax & syntax);

static void clear_default_type_argument_semantics_for_mangling(
    TemplateIdSyntax & syntax)
{
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    clear_default_type_argument_semantics_for_mangling(
        syntax.argument_syntaxes[i]);
  }
}

static void clear_default_type_argument_semantics_for_mangling(
    TemplateArgumentSyntax & syntax)
{
  syntax.resolved_type.reset();
  if(syntax.template_id) {
    clear_default_type_argument_semantics_for_mangling(*syntax.template_id);
  }
  if(syntax.type_id) {
    clear_default_type_argument_semantics_for_mangling(*syntax.type_id);
  }
  if(syntax.expression) {
    clear_default_type_argument_semantics_for_mangling(*syntax.expression);
  }
}

static void clear_default_type_argument_semantics_for_mangling(CppAstNode & node)
{
  node.semantic_type.reset();
  if(node.template_id_syntax) {
    clear_default_type_argument_semantics_for_mangling(
        *node.template_id_syntax);
  }
  if(node.conversion_type_id_syntax) {
    clear_default_type_argument_semantics_for_mangling(
        *node.conversion_type_id_syntax);
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    clear_default_type_argument_semantics_for_mangling(
        node.qualifier_template_id_syntaxes[i]);
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    clear_default_type_argument_semantics_for_mangling(
        node.qualifier_type_syntaxes[i]);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    clear_default_type_argument_semantics_for_mangling(node.children[i]);
  }
}

static TemplateArgumentSyntax default_template_argument_syntax_for_mangling(
    const TemplateParameterInfo & parameter,
    const CppAstNode & payload,
    const semantic_model::Scope * default_argument_scope)
{
  TemplateArgumentSyntax syntax;
  syntax.text = default_template_argument_text_for_mangling(parameter, payload);
  syntax.has_source_token_start = payload.token_end > payload.token_start;
  syntax.source_token_start = payload.token_start;
  syntax.source_location_id = payload.source_location_id;
  CppAstNode cloned_payload = clone_ast_node_for_mangling(payload);
  if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
    clear_default_type_argument_semantics_for_mangling(cloned_payload);
  }
  qualify_default_template_argument_node_for_mangling(cloned_payload,
                                                      default_argument_scope);
  if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
    syntax.expression.reset(new CppAstNode(std::move(cloned_payload)));
  } else {
    syntax.type_id.reset(new CppAstNode(std::move(cloned_payload)));
  }
  return syntax;
}

static vector<TemplateArgumentSyntax> explicit_alias_template_arguments_for_mangling(
    const TemplateIdSyntax & syntax)
{
  const size_t count =
      std::max(syntax.argument_syntaxes.size(), syntax.arguments.size());
  vector<TemplateArgumentSyntax> out;
  out.reserve(count);
  for(size_t i = 0; i < count; ++i) {
    TemplateArgumentSyntax argument;
    if(i < syntax.argument_syntaxes.size()) {
      argument = clone_template_argument_syntax_for_mangling(
          syntax.argument_syntaxes[i]);
    }
    if(argument.text.empty() && i < syntax.arguments.size()) {
      argument.text = syntax.arguments[i];
    }
    out.push_back(argument);
  }
  return out;
}

static bool append_default_alias_template_argument_for_mangling(
    const vector<TemplateParameterInfo> & parameters,
    size_t parameter_index,
    vector<TemplateArgumentSyntax> & arguments,
    const semantic_model::Scope * default_argument_scope)
{
  if(parameter_index >= parameters.size()) {
    return false;
  }
  const TemplateParameterInfo & parameter = parameters[parameter_index];
  const CppAstNode * payload = template_parameter_default_payload(parameter);
  if(!payload) {
    return false;
  }

  CppAstNode substituted = clone_ast_node_for_mangling(*payload);
  substitute_alias_template_arguments_in_node(substituted, parameters, arguments);
  arguments.push_back(
      default_template_argument_syntax_for_mangling(parameter,
                                                   substituted,
                                                   default_argument_scope));
  return true;
}

static void append_trailing_default_alias_template_arguments_for_mangling(
    const vector<TemplateParameterInfo> & parameters,
    vector<TemplateArgumentSyntax> & arguments,
    const semantic_model::Scope * default_argument_scope = nullptr)
{
  while(arguments.size() < parameters.size()) {
    const TemplateParameterInfo & parameter = parameters[arguments.size()];
    if(parameter.parameter_pack ||
       !parameter.default_argument ||
       !append_default_alias_template_argument_for_mangling(
           parameters, arguments.size(), arguments, default_argument_scope)) {
      break;
    }
  }
}

static vector<DependentAliasTemplateArgumentSyntax>
complete_dependent_alias_template_arguments_for_mangling(
    const vector<TemplateParameterInfo> & parameters,
    const vector<DependentAliasTemplateArgumentSyntax> & explicit_arguments,
    const semantic_model::Scope * default_argument_scope = nullptr)
{
  vector<TemplateArgumentSyntax> syntaxes;
  syntaxes.reserve(explicit_arguments.size());
  for(size_t i = 0; i < explicit_arguments.size(); ++i) {
    syntaxes.push_back(dependent_alias_argument_syntax_for_mangling(
        explicit_arguments[i]));
  }
  append_trailing_default_alias_template_arguments_for_mangling(parameters,
                                                               syntaxes,
                                                               default_argument_scope);

  vector<DependentAliasTemplateArgumentSyntax> out = explicit_arguments;
  out.reserve(syntaxes.size());
  for(size_t i = out.size(); i < syntaxes.size(); ++i) {
    DependentAliasTemplateArgumentSyntax argument;
    argument.syntax = syntaxes[i];
    argument.text = argument.syntax.text;
    if(argument.syntax.type_id && argument.syntax.type_id->semantic_type) {
      argument.type = argument.syntax.type_id->semantic_type;
    }
    out.push_back(argument);
  }
  return out;
}

static bool try_build_alias_template_id_expansion_syntax(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx,
    CppAstNode & out)
{
  const semantic_model::AliasTemplateDecl * alias_template =
      lookup_alias_template_for_template_id_syntax(syntax, mangle_ctx);
  vector<TemplateArgumentSyntax> arguments =
      explicit_alias_template_arguments_for_mangling(syntax);
  if(!alias_template || !alias_template->type_id ||
     arguments.size() > alias_template->parameters.size()) {
    return false;
  }
  append_trailing_default_alias_template_arguments_for_mangling(
      alias_template->parameters,
      arguments,
      alias_template->declaring_scope);

  out = clone_ast_node_for_mangling(*alias_template->type_id);
  substitute_alias_template_arguments_in_node(
      out,
      alias_template->parameters,
      arguments);
  return true;
}

static CppAstNode expand_alias_templates_in_type_ast_for_mangling_probe(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    size_t depth = 0)
{
  CppAstNode alias_expansion;
  if(depth <= 8 &&
     node.template_id_syntax &&
     lookup_alias_template_for_template_id_syntax(*node.template_id_syntax,
                                                  mangle_ctx) &&
     try_build_alias_template_id_expansion_syntax(*node.template_id_syntax,
                                                  mangle_ctx,
                                                  alias_expansion)) {
    return expand_alias_templates_in_type_ast_for_mangling_probe(
        alias_expansion,
        mangle_ctx,
        depth + 1);
  }

  CppAstNode out = clone_ast_node_for_mangling(node);
  for(size_t i = 0; i < out.children.size(); ++i) {
    out.children[i] =
        expand_alias_templates_in_type_ast_for_mangling_probe(
            out.children[i],
            mangle_ctx,
            depth);
  }
  return out;
}

static string mangle_ir_type_text_base(const string & raw);

static bool template_argument_syntax_mentions_direct_template_parameter(
    const TemplateArgumentSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx) {
    return false;
  }
  TypeMangleContext direct_ctx = *mangle_ctx;
  direct_ctx.owner_template_parameters = nullptr;
  direct_ctx.owner_template_arguments = nullptr;
  return template_argument_syntax_mentions_template_parameter(syntax,
                                                             &direct_ctx);
}

static bool template_argument_syntax_mentions_function_parameter(
    const TemplateArgumentSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(syntax.template_id) {
    for(size_t i = 0; i < syntax.template_id->argument_syntaxes.size(); ++i) {
      if(template_argument_syntax_mentions_function_parameter(
             syntax.template_id->argument_syntaxes[i],
             mangle_ctx)) {
        return true;
      }
    }
  }
  if(syntax.type_id &&
     ast_node_mentions_function_parameter(*syntax.type_id, mangle_ctx)) {
    return true;
  }
  return syntax.expression &&
         ast_node_mentions_function_parameter(*syntax.expression, mangle_ctx);
}

static bool template_id_syntax_is_enable_if(const TemplateIdSyntax & syntax)
{
  const string base_name =
      strip_leading_template_disambiguator(syntax.name.name);
  return base_name == "enable_if" || base_name == "enable_if_c";
}

static bool template_id_enable_if_condition_depends_on_function_template(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(!template_id_syntax_is_enable_if(syntax)) {
    return false;
  }
  if(!syntax.argument_syntaxes.empty()) {
    return template_argument_syntax_mentions_direct_template_parameter(
               syntax.argument_syntaxes[0],
               mangle_ctx) ||
           template_argument_syntax_mentions_function_parameter(
               syntax.argument_syntaxes[0],
               mangle_ctx);
  }
  return !syntax.arguments.empty() &&
         text_mentions_direct_template_mangle_parameter(syntax.arguments[0],
                                                        mangle_ctx);
}

struct EnableIfConditionDependency
{
  bool found = false;
  bool dependent = false;
};

static void collect_enable_if_condition_dependency(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx,
    EnableIfConditionDependency & out)
{
  if(!template_id_syntax_is_enable_if(syntax)) {
    return;
  }
  out.found = true;
  out.dependent =
      out.dependent ||
      template_id_enable_if_condition_depends_on_function_template(syntax,
                                                                   mangle_ctx);
}

static void collect_enable_if_condition_dependency(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    EnableIfConditionDependency & out)
{
  if(node.template_id_syntax) {
    collect_enable_if_condition_dependency(*node.template_id_syntax,
                                           mangle_ctx,
                                           out);
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    collect_enable_if_condition_dependency(node.qualifier_template_id_syntaxes[i],
                                           mangle_ctx,
                                           out);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_enable_if_condition_dependency(node.children[i], mangle_ctx, out);
  }
}

static EnableIfConditionDependency result_alias_enable_if_condition_dependency(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx)
{
  CppAstNode expanded =
      expand_alias_templates_in_type_ast_for_mangling_probe(node, mangle_ctx);
  EnableIfConditionDependency out;
  collect_enable_if_condition_dependency(expanded, mangle_ctx, out);
  return out;
}

static vector<TemplateArgument> template_arguments_for_dependent_mangling(
    const vector<DependentAliasTemplateArgumentSyntax> & arguments,
    const vector<TemplateParameterInfo> * parameters,
    const TypeMangleContext * mangle_ctx)
{
  vector<TemplateArgument> out;
  out.reserve(arguments.size());
  for(size_t i = 0; i < arguments.size(); ++i) {
    const TemplateParameterInfo * parameter =
        parameters && i < parameters->size() ? &(*parameters)[i] : nullptr;
    TemplateArgument arg;
    arg.text = arguments[i].text;
    TemplateArgumentSyntax syntax =
        dependent_alias_argument_syntax_for_mangling(arguments[i]);
    arg.source_syntax.reset(new TemplateArgumentSyntax(syntax));
    TypePtr argument_type = arguments[i].type;
    if(!argument_type && syntax.type_id && syntax.type_id->semantic_type) {
      argument_type = syntax.type_id->semantic_type;
    }
    if(!argument_type &&
       parameter &&
       template_argument_syntax_is_parameter_name(syntax, *parameter)) {
      argument_type = template_parameter_type_for_mangling(*parameter);
    }
    if(argument_type) {
      arg.kind = TemplateArgument::TA_TYPE;
      arg.type = argument_type;
    } else if(parameter &&
              parameter->kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
      arg.kind = TemplateArgument::TA_CLASS_TEMPLATE;
    } else {
      arg.kind = TemplateArgument::TA_VALUE;
      arg.expression = syntax.expression;
      const string value_text = trim_space(arguments[i].text);
      if(!value_text.empty() &&
         is_signed_decimal_integer_text(value_text)) {
        arg.dependent = false;
        arg.value = parse_signed_decimal_integer_value(value_text);
      } else {
      arg.dependent =
          template_argument_syntax_mentions_template_parameter(syntax,
                                                              mangle_ctx) ||
          (!arguments[i].text.empty() &&
           text_mentions_template_mangle_parameter(arguments[i].text, mangle_ctx));
      }
    }
    out.push_back(arg);
  }
  return out;
}

static const semantic_model::ClassInfo * nearest_member_class_scope(
    const semantic_model::Scope * scope)
{
  for(const semantic_model::Scope * current = scope; current; current = current->parent) {
    if(current->class_info &&
       current->class_info->member_scope.get() == current) {
      return current->class_info;
    }
  }
  return nullptr;
}

static TemplateArgumentSyntax effective_dependent_argument_syntax(
    const DependentAliasTemplateArgumentSyntax & argument)
{
  TemplateArgumentSyntax syntax = dependent_alias_argument_syntax_for_mangling(argument);
  if(syntax.text.empty()) {
    syntax.text = argument.text;
  }
  return syntax;
}

static string dependent_class_template_name_key_text(
    const string & prefix,
    const string & base_name,
    const vector<DependentAliasTemplateArgumentSyntax> & arguments)
{
  string out = append_qualified_component_text(prefix, trim_space(base_name));
  if(arguments.empty()) {
    return out;
  }

  out += '<';
  for(size_t i = 0; i < arguments.size(); ++i) {
    if(i != 0) {
      out += ", ";
    }
    string argument_text =
        trim_elaborated_type_prefix(arguments[i].text);
    if(argument_text.empty() && arguments[i].type) {
      argument_text =
          trim_elaborated_type_prefix(template_argument_type_text(arguments[i].type));
    }
    out += argument_text;
    if(arguments[i].syntax.pack_expansion &&
       out.size() >= 3 &&
       out.compare(out.size() - 3, 3, "...") != 0) {
      out += "...";
    }
  }
  out += '>';
  return out;
}

static bool dependent_class_template_metadata_should_drive_mangling(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx)
{
  if(!type || type->kind != Type::TK_NAMED) {
    return false;
  }
  const string selected_text = selected_named_type_text(type);
  if(mangle_ctx &&
     !selected_text.empty() &&
     selected_text.find('<') != string::npos &&
     !text_mentions_template_mangle_parameter(selected_text, mangle_ctx)) {
    return false;
  }
  if(named_type_has_dependent_semantic(type) ||
     named_type_key_contains_dependent_semantic(type)) {
    return true;
  }
  if(!mangle_ctx) {
    return true;
  }
  return selected_text.empty() ||
         text_mentions_template_mangle_parameter(selected_text, mangle_ctx);
}

static bool type_has_concrete_template_id_spelling_for_mangling(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx)
{
  const string selected_text = selected_named_type_text(type);
  return !selected_text.empty() &&
         selected_text.find('<') != string::npos &&
         !text_mentions_template_mangle_parameter(selected_text, mangle_ctx);
}

static bool is_mangleable_builtin_type_transform_name(const string & name)
{
  return builtin_type_transforms::is_supported_name(name);
}

static bool builtin_type_transform_ast_name(const CppAstNode & node,
                                            string & out)
{
  if(!node.builtin_type_transform_name.empty() &&
     is_mangleable_builtin_type_transform_name(
         node.builtin_type_transform_name)) {
    out = node.builtin_type_transform_name;
    return true;
  }

  return false;
}

static bool mangle_template_id_syntax_for_instantiation_match(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx,
    string & out)
{
  abi_mangle::Type type;
  if(!try_build_template_id_type_ir(syntax,
                                    mangle_ctx,
                                    type,
                                    false,
                                    true)) {
    return false;
  }
  out.clear();
  MangleSubstitutionState local_state;
  MangleIrSubstitutionSink sink(&local_state);
  return abi_mangle::emit_type(type, out, &sink);
}

static const semantic_model::ClassInfo * resolved_class_template_instantiation(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  const semantic_model::ClassTemplateDecl * class_template =
      lookup_class_template_for_template_id_syntax(syntax, mangle_ctx);
  if(!class_template) {
    return nullptr;
  }

  string syntax_mangle;
  if(!mangle_template_id_syntax_for_instantiation_match(
         syntax, mangle_ctx, syntax_mangle)) {
    return nullptr;
  }

  for(map<string, semantic_model::ClassInfo *>::const_iterator it =
          class_template->instantiations.begin();
      it != class_template->instantiations.end();
      ++it) {
    if(!it->second || !it->second->type) {
      continue;
    }
    MangleSubstitutionState local_state;
    string candidate_mangle;
    if(try_emit_type_encoding_ir_impl(it->second->type,
                            candidate_mangle,
                            mangle_ctx,
                            &local_state) &&
       candidate_mangle == syntax_mangle) {
      return it->second;
    }
  }
  for(map<string, semantic_model::ClassInfo *>::const_iterator it =
          class_template->reference_instantiations.begin();
      it != class_template->reference_instantiations.end();
      ++it) {
    if(!it->second || !it->second->type) {
      continue;
    }
    MangleSubstitutionState local_state;
    string candidate_mangle;
    if(try_emit_type_encoding_ir_impl(it->second->type,
                            candidate_mangle,
                            mangle_ctx,
                            &local_state) &&
       candidate_mangle == syntax_mangle) {
      return it->second;
    }
  }
  return nullptr;
}

static bool build_type_specifier_seq_ast_substitution_key(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    string & out);

static bool build_type_name_ast_substitution_key(const CppAstNode & node,
                                                 const TypeMangleContext * mangle_ctx,
                                                 string & out)
{
  if(try_mangle_builtin_text(node.value, out)) {
    out.clear();
    return false;
  }

  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  const TemplateIdSyntax * final_template_id = cppast_template_id_syntax(node);
  if(!final_template_id) {
    const string stripped = trim_elaborated_type_prefix(node.value);
    if(mangle_ctx &&
       mangle_ctx->owner_template_parameters &&
       mangle_ctx->owner_template_arguments) {
      const vector<TemplateParameterInfo> & owner_parameters =
          *mangle_ctx->owner_template_parameters;
      const vector<TemplateArgument> & owner_arguments =
          *mangle_ctx->owner_template_arguments;
      const vector<TemplateParameterInfo> * direct_parameters =
          mangle_ctx->template_parameters ?
              mangle_ctx->template_parameters->parameters :
              nullptr;
      if(!template_parameters_have_matching_type_name(direct_parameters, stripped)) {
        for(size_t i = 0; i < owner_parameters.size() && i < owner_arguments.size(); ++i) {
          const TemplateParameterInfo & parameter = owner_parameters[i];
          if(parameter.kind != TemplateParameterInfo::TP_TYPE ||
             parameter.name.empty() ||
             (!text_matches_type_parameter_name(stripped, parameter.name) &&
              stripped != parameter.placeholder_key)) {
            continue;
          }
          if(owner_template_argument_index_is_suppressed(mangle_ctx, i) ||
             template_argument_is_self_type_parameter(owner_arguments[i],
                                                      parameter,
                                                      TypePtr(),
                                                      stripped)) {
            out = template_parameter_type_substitution_key(
                &owner_parameters, i, parameter);
            return true;
          }
          return build_type_substitution_key_impl(
              owner_arguments[i].type,
              mangle_ctx,
              out,
              true);
        }
      }
    }
    if(mangle_ctx && mangle_ctx->template_parameters &&
       mangle_ctx->template_parameters->parameters) {
      const vector<TemplateParameterInfo> & parameters =
          *mangle_ctx->template_parameters->parameters;
      for(size_t i = 0; i < parameters.size(); ++i) {
        const TemplateParameterInfo & parameter = parameters[i];
        if(parameter.kind == TemplateParameterInfo::TP_TYPE &&
           !parameter.name.empty() &&
           text_matches_type_parameter_name(stripped, parameter.name)) {
          out = template_parameter_type_substitution_key(&parameters, i, parameter);
          return true;
        }
      }
    }

    if(mangle_ctx &&
       mangle_ctx->owner_template_parameters &&
       mangle_ctx->owner_template_arguments) {
      const vector<TemplateParameterInfo> & parameters =
          *mangle_ctx->owner_template_parameters;
      const vector<TemplateArgument> & arguments =
          *mangle_ctx->owner_template_arguments;
      for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
        const TemplateParameterInfo & parameter = parameters[i];
        const TemplateArgument & argument = arguments[i];
        if(parameter.kind != TemplateParameterInfo::TP_TYPE ||
           parameter.name.empty() ||
           argument.kind != TemplateArgument::TA_TYPE ||
           !argument.type) {
          continue;
        }
        if(text_matches_type_parameter_name(stripped, parameter.name) ||
           stripped == parameter.placeholder_key) {
          return build_type_substitution_key(argument.type, mangle_ctx, out);
        }
      }
    }

    TypePtr lookup_type = lookup_scope_named_type_for_mangling(node.value, mangle_ctx);
    if(lookup_type) {
      return build_type_substitution_key(lookup_type, mangle_ctx, out);
    }
    if(node.semantic_type &&
       build_type_substitution_key(node.semantic_type, mangle_ctx, out)) {
      return true;
    }
  }
  if(final_template_id) {
    string text;
    if(qualified && !qualified->qualifiers.empty()) {
      QualifiedName template_name = final_template_id->name;
      template_name.rooted = qualified->rooted;
      template_name.qualifiers = qualified->qualifiers;
      template_name.name =
          strip_leading_template_disambiguator(final_template_id->name.name);
      text = qualified_template_id_syntax_key_text(*final_template_id, template_name);
    } else {
      QualifiedName lookup_qualified;
      text = qualify_template_id_syntax_from_lookup(
                 *final_template_id, mangle_ctx, lookup_qualified) ?
                 qualified_template_id_syntax_key_text(*final_template_id,
                                                       lookup_qualified) :
                 template_id_syntax_key_text(*final_template_id);
    }
    out = named_substitution_key(text);
    return !out.empty();
  }
  if(qualified) {
    if(qualified->qualifiers.empty()) {
      return false;
    }
    out = named_substitution_key(qualified_name_syntax_key_text(*qualified));
    return !out.empty();
  }
  return false;
}

static bool build_type_specifier_seq_ast_substitution_key(const CppAstNode & node,
                                                          const TypeMangleContext * mangle_ctx,
                                                          string & out)
{
  if(node.kind != CppAstKind::decl_specifier_seq &&
     node.kind != CppAstKind::type_specifier_seq) {
    return build_type_name_ast_substitution_key(node, mangle_ctx, out);
  }

  const CppAstNode * type_node = nullptr;
  bool cv_const = false;
  bool cv_volatile = false;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.value == "const") {
      cv_const = true;
      continue;
    }
    if(child.value == "volatile") {
      cv_volatile = true;
      continue;
    }
    if(child.value == "typename" || child.value == "class" || child.value == "struct") {
      continue;
    }
    type_node = &child;
  }
  if(!type_node ||
     !build_type_name_ast_substitution_key(*type_node, mangle_ctx, out)) {
    return false;
  }
  if(cv_const || cv_volatile) {
    out = string("type:cv(") + (cv_const ? "K" : "") +
          (cv_volatile ? "V" : "") + "," + out + ")";
  }
  return true;
}

static bool try_mangle_non_type_template_parameter_expression_name(
    const string & text,
    string & out,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state)
{
  if(!mangle_ctx) {
    return false;
  }

  const string stripped = trim_elaborated_type_prefix(text);
  if(mangle_ctx->template_parameters &&
     mangle_ctx->template_parameters->parameters) {
    const vector<TemplateParameterInfo> & parameters =
        *mangle_ctx->template_parameters->parameters;
    for(size_t i = 0; i < parameters.size(); ++i) {
      const TemplateParameterInfo & parameter = parameters[i];
      if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE ||
         parameter.name.empty() ||
         stripped != parameter.name) {
        continue;
      }
      return emit_template_parameter_expression_index(
          i, parameter, out, state, false);
    }
  }

  if(mangle_ctx->owner_template_parameters &&
     mangle_ctx->owner_template_arguments) {
    const vector<TemplateParameterInfo> & parameters =
        *mangle_ctx->owner_template_parameters;
    const vector<TemplateArgument> & arguments =
        *mangle_ctx->owner_template_arguments;
    for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
      const TemplateParameterInfo & parameter = parameters[i];
      if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE ||
         parameter.name.empty() ||
         stripped != parameter.name) {
        continue;
      }
      if(arguments[i].kind == TemplateArgument::TA_VALUE &&
         arguments[i].dependent &&
         trim_space(arguments[i].text) == parameter.name) {
        return emit_template_parameter_expression_index(
            i, parameter, out, state, false);
      }
      return try_mangle_template_argument_impl(
          arguments[i], &parameter, out, mangle_ctx, state);
    }
  }
  return false;
}

static bool try_mangle_non_type_template_parameter_expression_pack_text(
    const string & text,
    string & out,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state)
{
  string stripped = trim_space(strip_elaborated_type_prefix(text));
  if(stripped.size() < 3 ||
     stripped.compare(stripped.size() - 3, 3, "...") != 0) {
    return false;
  }
  stripped = trim_space(stripped.substr(0, stripped.size() - 3));
  if(stripped.empty()) {
    return false;
  }

  const size_t begin = out.size();
  out += "Xsp";
  if(!try_mangle_non_type_template_parameter_expression_name(
         stripped, out, mangle_ctx, state)) {
    out.resize(begin);
    return false;
  }
  out += 'E';
  return true;
}

static bool try_build_array_bound_ast_encoding_text(
    const CppAstNode & node,
    string & out,
    const TypeMangleContext * mangle_ctx)
{
  if(node.kind == CppAstKind::literal && is_signed_decimal_integer_text(node.value)) {
    out += node.value[0] == '+' ? node.value.substr(1) : node.value;
    return true;
  }
  if((node.kind == CppAstKind::id_expression ||
      node.kind == CppAstKind::identifier) &&
     try_mangle_non_type_template_parameter_expression_name(
         node.value, out, mangle_ctx, nullptr)) {
    return true;
  }
  return false;
}

static bool build_template_parameter_index_expression_key(
    const string & text,
    const TypeMangleContext * mangle_ctx,
    string & out)
{
  if(!mangle_ctx || !mangle_ctx->template_parameters ||
     !mangle_ctx->template_parameters->parameters) {
    return false;
  }

  const string stripped = trim_elaborated_type_prefix(text);
  const vector<TemplateParameterInfo> & parameters =
      *mangle_ctx->template_parameters->parameters;
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].name.empty() && stripped == parameters[i].name) {
      out = string("expr:tparam(") + parameters[i].name + ")";
      return true;
    }
  }
  return false;
}

static bool build_array_bound_ast_key(const CppAstNode & node,
                                      const TypeMangleContext * mangle_ctx,
                                      string & out)
{
  if(node.kind == CppAstKind::literal && is_signed_decimal_integer_text(node.value)) {
    out = string("expr:int(") +
          (node.value[0] == '+' ? node.value.substr(1) : node.value) + ")";
    return true;
  }
  if((node.kind == CppAstKind::id_expression ||
      node.kind == CppAstKind::identifier) &&
     build_template_parameter_index_expression_key(node.value, mangle_ctx, out)) {
    return true;
  }
  return false;
}

static bool build_parameter_declaration_ast_substitution_key(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    string & out);

static bool build_parameter_clause_ast_type_key(const CppAstNode & clause,
                                                const string & return_key,
                                                const TypeMangleContext * mangle_ctx,
                                                string & out)
{
  if(clause.kind != CppAstKind::parameter_clause || return_key.empty()) {
    return false;
  }

  ostringstream key;
  key << "type:fn(" << return_key << ";";
  bool variadic = false;
  size_t param_count = 0;
  for(size_t i = 0; i < clause.children.size(); ++i) {
    const CppAstNode & child = clause.children[i];
    if(child.kind == CppAstKind::ellipsis) {
      variadic = true;
      continue;
    }
    if(child.kind != CppAstKind::parameter_declaration) {
      continue;
    }
    string param_key;
    if(!build_parameter_declaration_ast_substitution_key(
           child, mangle_ctx, param_key)) {
      return false;
    }
    if(param_count != 0) {
      key << ",";
    }
    key << param_key;
    ++param_count;
  }
  key << ";";
  key << (variadic ? "z" : "v");
  key << ")";
  out = key.str();
  return true;
}

static bool build_declarator_ast_substitution_key_impl(
    const CppAstNode & declarator,
    const TypeMangleContext * mangle_ctx,
    const function<bool(string &)> & build_base,
    string & out)
{
  if(declarator.kind != CppAstKind::declarator &&
     declarator.kind != CppAstKind::abstract_declarator) {
    return build_base(out);
  }

  vector<const CppAstNode *> ptr_operators;
  vector<const CppAstNode *> suffixes;
  const CppAstNode * nested_declarator = nullptr;
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    const CppAstNode & child = declarator.children[i];
    if(child.kind == CppAstKind::ptr_operator) {
      ptr_operators.push_back(&child);
    } else if(child.kind == CppAstKind::array_suffix ||
              child.kind == CppAstKind::parameter_clause) {
      suffixes.push_back(&child);
    } else if(child.kind == CppAstKind::nested_declarator) {
      for(size_t j = 0; j < child.children.size(); ++j) {
        if(child.children[j].kind == CppAstKind::declarator ||
           child.children[j].kind == CppAstKind::abstract_declarator) {
          nested_declarator = &child.children[j];
          break;
        }
      }
    }
  }

  const function<bool(string &)> build_pointer_base =
      [&ptr_operators, &build_base](string & key) -> bool
      {
        if(!build_base(key)) {
          return false;
        }
        for(vector<const CppAstNode *>::const_reverse_iterator it =
                ptr_operators.rbegin();
            it != ptr_operators.rend();
            ++it) {
          const CppAstNode & op = **it;
          if(op.has_token && op.simple_type == OP_STAR) {
            key = string("type:ptr(") + key + ")";
          } else if(op.has_token && op.simple_type == OP_AMP) {
            key = string("type:lref(") + key + ")";
          } else if(op.has_token && op.simple_type == OP_LAND) {
            key = string("type:rref(") + key + ")";
          } else {
            return false;
          }
        }
        return true;
      };

  function<bool(size_t, string &)> build_suffix_key =
      [&](size_t index, string & key) -> bool
      {
        if(index == suffixes.size()) {
          return build_pointer_base(key);
        }

        const CppAstNode & suffix = *suffixes[index];
        string inner_key;
        if(!build_suffix_key(index + 1, inner_key)) {
          return false;
        }
        if(suffix.kind == CppAstKind::array_suffix) {
          string bound_key;
          if(suffix.children.empty()) {
            bound_key = "expr:unknown";
          } else if(!build_array_bound_ast_key(
                        suffix.children[0], mangle_ctx, bound_key)) {
            return false;
          }
          key = string("type:array(") + bound_key + "," + inner_key + ")";
          return true;
        }
        if(suffix.kind == CppAstKind::parameter_clause) {
          return build_parameter_clause_ast_type_key(
              suffix, inner_key, mangle_ctx, key);
        }
        return false;
      };

  const function<bool(string &)> build_current =
      [&build_suffix_key](string & key) -> bool
      {
        return build_suffix_key(0, key);
      };

  if(nested_declarator) {
    return build_declarator_ast_substitution_key_impl(
        *nested_declarator, mangle_ctx, build_current, out);
  }
  return build_current(out);
}

static bool build_declarator_ast_substitution_key(
    const CppAstNode & declarator,
    const string & specifier_key,
    const TypeMangleContext * mangle_ctx,
    string & out)
{
  return build_declarator_ast_substitution_key_impl(
      declarator,
      mangle_ctx,
      [&specifier_key](string & key) -> bool
      {
        key = specifier_key;
        return !key.empty();
      },
      out);
}

static bool build_parameter_declaration_ast_substitution_key(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    string & out)
{
  if(node.kind != CppAstKind::parameter_declaration) {
    return false;
  }

  const CppAstNode * specifiers = nullptr;
  const CppAstNode * declarator = nullptr;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::decl_specifier_seq ||
       child.kind == CppAstKind::type_specifier_seq) {
      specifiers = &child;
    } else if(child.kind == CppAstKind::declarator ||
              child.kind == CppAstKind::abstract_declarator) {
      declarator = &child;
    }
  }
  if(!specifiers ||
     !build_type_specifier_seq_ast_substitution_key(
         *specifiers, mangle_ctx, out)) {
    return false;
  }
  if(!declarator) {
    return true;
  }
  return build_declarator_ast_substitution_key(
      *declarator, out, mangle_ctx, out);
}

static bool simple_reference_declarator_operator(const CppAstNode & declarator,
                                                 ETokenType & out_operator)
{
  if(declarator.kind != CppAstKind::declarator &&
     declarator.kind != CppAstKind::abstract_declarator) {
    return false;
  }

  bool found_reference = false;
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    const CppAstNode & child = declarator.children[i];
    if(child.kind == CppAstKind::ptr_operator) {
      if(found_reference ||
         !child.has_token ||
         (child.simple_type != OP_AMP && child.simple_type != OP_LAND)) {
        return false;
      }
      found_reference = true;
      out_operator = child.simple_type;
      continue;
    }
    if(child.kind == CppAstKind::array_suffix ||
       child.kind == CppAstKind::parameter_clause ||
       child.kind == CppAstKind::nested_declarator) {
      return false;
    }
  }
  return found_reference;
}

static bool specifier_seq_names_owner_template_type_parameter(
    const CppAstNode & specifiers,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx ||
     !mangle_ctx->owner_template_parameters) {
    return false;
  }

  const CppAstNode * type_node = nullptr;
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    const CppAstNode & child = specifiers.children[i];
    if(child.value == "const" ||
       child.value == "volatile" ||
       child.value == "typename" ||
       child.value == "class" ||
       child.value == "struct") {
      continue;
    }
    if(type_node) {
      return false;
    }
    type_node = &child;
  }
  if(!type_node) {
    return false;
  }

  const string stripped =
      trim_elaborated_type_prefix(type_node->value);
  const vector<TemplateParameterInfo> & parameters =
      *mangle_ctx->owner_template_parameters;
  for(size_t i = 0; i < parameters.size(); ++i) {
    const TemplateParameterInfo & parameter = parameters[i];
    if(parameter.kind != TemplateParameterInfo::TP_TYPE ||
       parameter.name.empty()) {
      continue;
    }
    if(text_matches_type_parameter_name(stripped, parameter.name) ||
       stripped == parameter.placeholder_key) {
      return true;
    }
  }
  return false;
}

static bool owner_template_type_argument_for_specifier_seq(
    const CppAstNode & specifiers,
    const TypeMangleContext * mangle_ctx,
    TypePtr & out_type)
{
  if(!mangle_ctx ||
     !mangle_ctx->owner_template_parameters ||
     !mangle_ctx->owner_template_arguments) {
    return false;
  }

  const CppAstNode * type_node = nullptr;
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    const CppAstNode & child = specifiers.children[i];
    if(child.value == "const" ||
       child.value == "volatile" ||
       child.value == "typename" ||
       child.value == "class" ||
       child.value == "struct") {
      continue;
    }
    if(type_node) {
      return false;
    }
    type_node = &child;
  }
  if(!type_node) {
    return false;
  }

  const string stripped =
      trim_elaborated_type_prefix(type_node->value);
  const vector<TemplateParameterInfo> & parameters =
      *mangle_ctx->owner_template_parameters;
  const vector<TemplateArgument> & arguments =
      *mangle_ctx->owner_template_arguments;
  for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
    const TemplateParameterInfo & parameter = parameters[i];
    const TemplateArgument & argument = arguments[i];
    if(parameter.kind != TemplateParameterInfo::TP_TYPE ||
       parameter.name.empty() ||
       argument.kind != TemplateArgument::TA_TYPE ||
       !argument.type) {
      continue;
    }
    if(text_matches_type_parameter_name(stripped, parameter.name) ||
       stripped == parameter.placeholder_key) {
      out_type = argument.type;
      return true;
    }
  }
  return false;
}

static void specifier_seq_cv_qualifiers(const CppAstNode & specifiers,
                                        bool & cv_const,
                                        bool & cv_volatile)
{
  cv_const = false;
  cv_volatile = false;
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    const CppAstNode & child = specifiers.children[i];
    if(child.value == "const") {
      cv_const = true;
    } else if(child.value == "volatile") {
      cv_volatile = true;
    }
  }
}

static bool try_build_actual_owner_template_reference_type_id_ast_ir(
    const CppAstNode & node,
    const TypePtr & actual_type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(node.kind != CppAstKind::type_id || node.children.empty() || !actual_type) {
    return false;
  }

  const CppAstNode * abstract = nullptr;
  for(size_t i = 1; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::abstract_declarator) {
      abstract = &node.children[i];
      break;
    }
  }
  if(!abstract) {
    return false;
  }

  ETokenType reference_operator = static_cast<ETokenType>(0);
  if(!simple_reference_declarator_operator(*abstract, reference_operator)) {
    return false;
  }

  TypePtr owner_argument_type;
  if(owner_template_type_argument_for_specifier_seq(node.children[0],
                                                    mangle_ctx,
                                                    owner_argument_type)) {
    bool cv_const = false;
    bool cv_volatile = false;
    specifier_seq_cv_qualifiers(node.children[0], cv_const, cv_volatile);
    TypePtr base = apply_cv(owner_argument_type, cv_const, cv_volatile);
    TypePtr reference_type =
        reference_operator == OP_AMP ?
            make_lvalue_reference_raw(base) :
            make_rvalue_reference_raw(base);
    return try_build_type_ir(reference_type, mangle_ctx, out);
  }

  if(!specifier_seq_names_owner_template_type_parameter(
         node.children[0], mangle_ctx)) {
    return false;
  }

  return try_build_type_ir(actual_type, mangle_ctx, out);
}

static const CppAstNode * template_parameter_default_payload(
    const TemplateParameterInfo & parameter)
{
  if(!parameter.default_argument) {
    return nullptr;
  }
  return !parameter.default_argument->children.empty() ?
      &parameter.default_argument->children[0] :
      parameter.default_argument;
}

static string non_type_template_parameter_decl_specifier_text(
    const TemplateParameterInfo & parameter);
static bool type_has_dependent_mangle_state(const TypePtr & type);

static bool build_type_id_ir_from_ast(
    const CppAstNode & type_id,
    const TypePtr & actual_type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(!try_build_actual_owner_template_reference_type_id_ast_ir(
         type_id, actual_type, mangle_ctx, out) &&
     !try_build_type_id_ast_ir(type_id, mangle_ctx, out)) {
    return false;
  }
  return true;
}

static bool build_parameter_declaration_ir_from_ast(
    const CppAstNode & declaration,
    const TypePtr & actual_type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(declaration.kind != CppAstKind::parameter_declaration) {
    return false;
  }

  const CppAstNode * specifiers = nullptr;
  const CppAstNode * declarator = nullptr;
  for(size_t i = 0; i < declaration.children.size(); ++i) {
    const CppAstNode & child = declaration.children[i];
    if(child.kind == CppAstKind::decl_specifier_seq ||
       child.kind == CppAstKind::type_specifier_seq) {
      specifiers = &child;
    } else if(child.kind == CppAstKind::declarator ||
              child.kind == CppAstKind::abstract_declarator) {
      declarator = &child;
    }
  }
  if(!specifiers) {
    return false;
  }

  CppAstNode type_id;
  type_id.kind = CppAstKind::type_id;
  type_id.value = declaration.value;
  type_id.children.push_back(*specifiers);
  if(declarator) {
    CppAstNode abstract = *declarator;
    abstract.kind = CppAstKind::abstract_declarator;
    type_id.children.push_back(abstract);
  }
  return build_type_id_ir_from_ast(type_id, actual_type, mangle_ctx, out);
}

static bool token_or_text_is(const CppAstNode & node, ETokenType token, const char * text)
{
  return (node.has_token &&
          node.token_kind == RT_SIMPLE &&
          node.simple_type == token) ||
         trim_space(node.value) == text;
}

static bool try_build_fundamental_type_specifier_seq_ast_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(node.kind != CppAstKind::decl_specifier_seq &&
     node.kind != CppAstKind::type_specifier_seq) {
    return false;
  }

  bool cv_const = false;
  bool cv_volatile = false;
  int signed_count = 0;
  int unsigned_count = 0;
  int short_count = 0;
  int long_count = 0;
  bool saw_int = false;
  bool saw_char = false;
  bool saw_char16 = false;
  bool saw_char32 = false;
  bool saw_wchar = false;
  bool saw_bool = false;
  bool saw_float = false;
  bool saw_double = false;
  bool saw_void = false;

  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(token_or_text_is(child, KW_CONST, "const")) {
      cv_const = true;
    } else if(token_or_text_is(child, KW_VOLATILE, "volatile")) {
      cv_volatile = true;
    } else if(token_or_text_is(child, KW_SIGNED, "signed")) {
      ++signed_count;
    } else if(token_or_text_is(child, KW_UNSIGNED, "unsigned")) {
      ++unsigned_count;
    } else if(token_or_text_is(child, KW_SHORT, "short")) {
      ++short_count;
    } else if(token_or_text_is(child, KW_LONG, "long")) {
      ++long_count;
    } else if(token_or_text_is(child, KW_INT, "int")) {
      saw_int = true;
    } else if(token_or_text_is(child, KW_CHAR, "char")) {
      saw_char = true;
    } else if(token_or_text_is(child, KW_CHAR16_T, "char16_t")) {
      saw_char16 = true;
    } else if(token_or_text_is(child, KW_CHAR32_T, "char32_t")) {
      saw_char32 = true;
    } else if(token_or_text_is(child, KW_WCHAR_T, "wchar_t")) {
      saw_wchar = true;
    } else if(token_or_text_is(child, KW_BOOL, "bool")) {
      saw_bool = true;
    } else if(token_or_text_is(child, KW_FLOAT, "float")) {
      saw_float = true;
    } else if(token_or_text_is(child, KW_DOUBLE, "double")) {
      saw_double = true;
    } else if(token_or_text_is(child, KW_VOID, "void")) {
      saw_void = true;
    } else {
      return false;
    }
  }

  TypePtr fundamental;
  if(!finalize_fundamental_type_specifiers(signed_count,
                                           unsigned_count,
                                           short_count,
                                           long_count,
                                           saw_int,
                                           saw_char,
                                           saw_char16,
                                           saw_char32,
                                           saw_wchar,
                                           saw_bool,
                                           saw_float,
                                           saw_double,
                                           saw_void,
                                           fundamental)) {
    return false;
  }

  TypePtr typed = apply_cv(fundamental, cv_const, cv_volatile);
  return try_build_type_ir(typed, mangle_ctx, out);
}

static bool emit_type_ir_owned(abi_mangle::Type & type,
                               MangleSubstitutionState * state,
                               string & out)
{
  MangleIrSubstitutionSink sink(state);
  return abi_mangle::emit_type_owned(type, out, &sink);
}

static bool emit_type_ir(const abi_mangle::Type & type,
                         MangleSubstitutionState * state,
                         string & out)
{
  MangleIrSubstitutionSink sink(state);
  return abi_mangle::emit_type(type, out, &sink);
}

static bool type_ir_can_be_emitted(const abi_mangle::Type & type)
{
  MangleSubstitutionState state;
  string ignored;
  return emit_type_ir(type, &state, ignored) && !ignored.empty();
}

static bool build_and_emit_type_ir(const TypePtr & type,
                                   const TypeMangleContext * mangle_ctx,
                                   MangleSubstitutionState * state,
                                   string & out,
                                   abi_mangle::Type * captured_type = nullptr)
{
  abi_mangle::Type ir_type;
  if(!try_build_type_ir(type, mangle_ctx, ir_type)) {
    return false;
  }
  if(captured_type) {
    if(!emit_type_ir(ir_type, state, out)) {
      return false;
    }
    *captured_type = ir_type;
    return true;
  }
  if(!emit_type_ir_owned(ir_type, state, out)) {
    return false;
  }
  return true;
}

static bool build_and_emit_type_id_ir_from_ast(
    const CppAstNode & type_id,
    const TypePtr & actual_type,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state,
    string & out,
    abi_mangle::Type * captured_type = nullptr)
{
  abi_mangle::Type ir_type;
  if(!build_type_id_ir_from_ast(type_id, actual_type, mangle_ctx, ir_type)) {
    return false;
  }
  if(captured_type) {
    if(!emit_type_ir(ir_type, state, out)) {
      return false;
    }
    *captured_type = ir_type;
    return true;
  }
  if(!emit_type_ir_owned(ir_type, state, out)) {
    return false;
  }
  return true;
}

static bool build_and_emit_parameter_declaration_ir_from_ast(
    const CppAstNode & declaration,
    const TypePtr & actual_type,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state,
    string & out,
    abi_mangle::Type * captured_type = nullptr)
{
  abi_mangle::Type ir_type;
  if(!build_parameter_declaration_ir_from_ast(declaration,
                                              actual_type,
                                              mangle_ctx,
                                              ir_type)) {
    return false;
  }
  if(captured_type) {
    if(!emit_type_ir(ir_type, state, out)) {
      return false;
    }
    *captured_type = ir_type;
    return true;
  }
  if(!emit_type_ir_owned(ir_type, state, out)) {
    return false;
  }
  return true;
}

static bool try_mangle_dependent_expression_ast_template_argument(
    const CppAstNode & node,
    string & out,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state)
{
  if(node.kind == CppAstKind::id_expression ||
     node.kind == CppAstKind::identifier) {
    abi_mangle::Type::ClassTemplateArgument owner_argument;
    if(try_build_owner_non_type_template_argument_ir(node.value,
                                                     false,
                                                     mangle_ctx,
                                                     owner_argument)) {
      MangleIrSubstitutionSink sink(state);
      return abi_mangle::emit_class_template_argument(owner_argument,
                                                             out,
                                                             &sink);
    }
  }

  abi_mangle::DependentExpression expression;
  if(!try_build_dependent_expression_ir(node, mangle_ctx, expression)) {
    return false;
  }
	  MangleIrSubstitutionSink sink(state);
	  return abi_mangle::emit_template_argument(
	      abi_mangle::TemplateArgument::dependent_expression_arg(
	          std::move(expression)),
      out,
      &sink);
}

static bool finish_mangle_template_argument_syntax_result(
    const TemplateArgumentSyntax & syntax,
    bool ok,
    MangleSubstitutionState * state)
{
  if(ok && syntax.pack_expansion) {
    register_substitution_key(
        state,
        string("template-arg-pack(") + trim_space(syntax.text) + ")");
  }
  return ok;
}

static bool try_mangle_template_argument_syntax_impl(
    const TemplateArgumentSyntax & syntax,
    const TemplateParameterInfo * parameter,
    string & out,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state)
{
  abi_mangle::Type::ClassTemplateArgument argument;
  if(!try_build_template_argument_syntax_ir(syntax,
                                            parameter,
                                            mangle_ctx,
                                            argument)) {
    return false;
  }
  MangleIrSubstitutionSink sink(state);
  return finish_mangle_template_argument_syntax_result(
      syntax,
      abi_mangle::emit_class_template_argument(argument, out, &sink),
      state);
}

static bool build_type_substitution_key(const TypePtr & type,
                                        const TypeMangleContext * mangle_ctx,
                                        string & out);

static bool named_type_uses_direct_std_standard_substitution(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx)
{
  if(!type || type->kind != Type::TK_NAMED) {
    return false;
  }
  shared_ptr<const ClassTemplateSpecializationMangleInfo> info =
      named_type_class_template_specialization_mangle_info_const(type);
  if(!info) {
    return false;
  }
  string canonical_scope;
  if(!canonicalize_named_substitution_text(info->template_scope_prefix,
                                           canonical_scope) ||
     canonical_scope != "std") {
    return false;
  }
  string code;
  bool substitution_includes_arguments = false;
  return structured_std_standard_substitution_for_template_component(
             trim_space(info->template_name),
             info->arguments,
             canonical_scope,
             mangle_ctx,
             code,
             substitution_includes_arguments) &&
         !code.empty();
}

static void register_direct_std_type_substitution_if_needed(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state)
{
  if(!direct_std_standard_substitutions_enabled(mangle_ctx)) {
    return;
  }
  string substitution_key;
  if(named_type_uses_direct_std_standard_substitution(type, mangle_ctx) &&
     build_type_substitution_key(type, mangle_ctx, substitution_key)) {
    register_substitution_key(state, substitution_key);
  }
}

static bool append_fundamental_mangle_code(EFundamentalType fundamental, string & out)
{
  switch(fundamental) {
  case FT_SIGNED_CHAR:
    out += 'a';
    return true;
  case FT_SHORT_INT:
    out += 's';
    return true;
  case FT_INT:
    out += 'i';
    return true;
  case FT_LONG_INT:
    out += 'l';
    return true;
  case FT_LONG_LONG_INT:
    out += 'x';
    return true;
  case FT_INT128:
    out += 'n';
    return true;
  case FT_UNSIGNED_CHAR:
    out += 'h';
    return true;
  case FT_UNSIGNED_SHORT_INT:
    out += 't';
    return true;
  case FT_UNSIGNED_INT:
    out += 'j';
    return true;
  case FT_UNSIGNED_LONG_INT:
    out += 'm';
    return true;
  case FT_UNSIGNED_LONG_LONG_INT:
    out += 'y';
    return true;
  case FT_UINT128:
    out += 'o';
    return true;
  case FT_WCHAR_T:
    out += 'w';
    return true;
  case FT_CHAR:
    out += 'c';
    return true;
  case FT_CHAR16_T:
    out += "Ds";
    return true;
  case FT_CHAR32_T:
    out += "Di";
    return true;
  case FT_BOOL:
    out += 'b';
    return true;
  case FT_FLOAT:
    out += 'f';
    return true;
  case FT_DOUBLE:
    out += 'd';
    return true;
  case FT_LONG_DOUBLE:
    out += 'e';
    return true;
  case FT_VOID:
    out += 'v';
    return true;
  case FT_NULLPTR_T:
    out += "Dn";
    return true;
  }
  return false;
}

static bool build_fundamental_substitution_atom_key(EFundamentalType fundamental, string & out)
{
  string code;
  if(!append_fundamental_mangle_code(fundamental, code) || code.empty()) {
    return false;
  }
  out = string("type:builtin(") + code + ")";
  return true;
}

static bool build_array_bound_type_mangle(const TypePtr & type,
                                          string & out);
static bool build_array_bound_substitution_key(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    string & out);
static bool should_prefer_unqualified_lexical_named_type(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx);

static bool context_free_type_ir_only_context(
    const TypeMangleContext * mangle_ctx)
{
  return mangle_ctx &&
         mangle_ctx->suppress_type_substitution_keys &&
         !mangle_ctx->template_parameters &&
         !mangle_ctx->owner_template_parameters &&
         !mangle_ctx->owner_template_arguments &&
         mangle_ctx->suppressed_owner_template_argument_indices.empty() &&
         !mangle_ctx->function_parameters &&
         mangle_ctx->lexical_scope.empty() &&
         !mangle_ctx->lookup_scope &&
         mangle_ctx->allow_direct_std_standard_substitutions &&
         !mangle_ctx->suppress_template_argument_pack_grouping &&
         !mangle_ctx->suppress_template_parameter_type_registration &&
         !mangle_ctx->template_arguments &&
         !mangle_ctx->template_argument_pack_sizes &&
         !mangle_ctx->prefer_template_argument_values &&
         !mangle_ctx->prefer_concrete_non_type_values_for_dependent_parameter_types &&
         mangle_ctx->alias_expansion_depth == 0 &&
         !mangle_ctx->canonical_enable_if_result_alias_substitutions &&
         !mangle_ctx->suppress_current_type_id_substitution_registration &&
         !mangle_ctx->suppress_expression_qualifier_template_name_substitution &&
         !mangle_ctx->suppress_member_template_name_component_substitution &&
         !mangle_ctx->suppress_decltype_callee_template_prefix_substitution &&
         !mangle_ctx->prefer_source_template_parameter_expression_arguments &&
         !mangle_ctx->prefer_source_template_name_prefixes_in_expressions;
}

static bool attach_context_free_type_ir_substitution(
    abi_mangle::Type & ir_type)
{
  abi_mangle::SubstitutionKey substitution_key;
  if(abi_mangle::make_type_substitution_key(ir_type, substitution_key) &&
     !substitution_key.empty()) {
    abi_mangle::set_substitution(ir_type, std::move(substitution_key));
  }
  return true;
}

static bool attach_type_ir_substitution(abi_mangle::Type & ir_type)
{
  abi_mangle::SubstitutionKey substitution_key;
  if(!abi_mangle::make_type_substitution_key(ir_type, substitution_key) ||
     substitution_key.empty()) {
    return false;
  }
  abi_mangle::set_substitution(ir_type, std::move(substitution_key));
  return true;
}

static bool attach_semantic_type_ir_substitution(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & ir_type)
{
  if(mangle_ctx && mangle_ctx->suppress_type_substitution_keys) {
    return true;
  }

  if(attach_type_ir_substitution(ir_type)) {
    return true;
  }

  abi_mangle::SubstitutionKey semantic_key;
  if(build_structural_type_substitution_key(type,
                                            mangle_ctx,
                                            semantic_key,
                                            false) &&
     !semantic_key.empty()) {
    abi_mangle::set_substitution(ir_type, std::move(semantic_key));
    return true;
  }
  return attach_context_free_type_ir_substitution(ir_type);
}

static void apply_function_type_cv_ir(const TypePtr & type,
                                      abi_mangle::Type & ir_type)
{
  if(!type ||
     type->kind != Type::TK_FUNCTION ||
     (!type->function_const && !type->function_volatile)) {
    return;
  }
  ir_type = abi_mangle::Type::cv(type->function_const,
                                 type->function_volatile,
                                 std::move(ir_type));
}

static void apply_function_type_cv_substitution_key(
    const TypePtr & type,
    abi_mangle::SubstitutionKey & key)
{
  if(!type ||
     type->kind != Type::TK_FUNCTION ||
     (!type->function_const && !type->function_volatile)) {
    return;
  }
  key = abi_mangle::SubstitutionKey::type_cv(type->function_const,
                                             type->function_volatile,
                                             std::move(key));
}

static bool attach_member_type_ir_substitution(
    abi_mangle::Type & ir_type,
    const string & source_text)
{
  string flat_text = trim_elaborated_type_prefix(source_text);
  static const char typename_prefix[] = "typename ";
  if(flat_text.compare(0, sizeof(typename_prefix) - 1, typename_prefix) == 0) {
    flat_text = trim_space(flat_text.substr(sizeof(typename_prefix) - 1));
  }
  if(!flat_text.empty() &&
     flat_text.find("::") != string::npos &&
     text_uses_only_simple_mangleable_chars(flat_text)) {
    abi_mangle::set_substitution(
        ir_type,
        abi_mangle::SubstitutionKey::named(flat_text));
    return true;
  }

  if(!ir_type.name_owner) {
    return false;
  }

  abi_mangle::SubstitutionKey owner_key;
  if(!abi_mangle::make_type_substitution_key(*ir_type.name_owner,
                                                    owner_key)) {
    return false;
  }

  string payload;
  const abi_mangle::Type::NameMetadata * name_metadata =
      ir_type.name.get();
  if(ir_type.kind == abi_mangle::Type::TK_NAMED) {
    if(!name_metadata || name_metadata->template_name.empty()) {
      return false;
    }
    payload = string("member-named:") + owner_key.structural_text() + "::" +
              name_metadata->template_name;
  } else if(ir_type.kind ==
            abi_mangle::Type::TK_CLASS_TEMPLATE_SPECIALIZATION) {
    if(!name_metadata || name_metadata->template_name.empty()) {
      return false;
    }
    abi_mangle::Type::ensure_name_metadata(ir_type)
        .template_name_ir_substitution =
            abi_mangle::SubstitutionKey::type(
                string("member-template-prefix:") +
                owner_key.structural_text() + "::" +
                name_metadata->template_name);
    payload = string("member-template:") + owner_key.structural_text() + "::" +
              name_metadata->template_name + "<";
    for(size_t i = 0; i < name_metadata->template_arguments.size(); ++i) {
      abi_mangle::SubstitutionKey argument_key;
      if(!abi_mangle::make_class_template_argument_substitution_key(
             name_metadata->template_arguments[i],
             argument_key)) {
        return false;
      }
      if(i != 0) {
        payload += ',';
      }
      payload += argument_key.structural_text();
    }
    payload += '>';
  } else {
    return false;
  }

  abi_mangle::SubstitutionKey structural_key =
      abi_mangle::SubstitutionKey::type(payload);
  abi_mangle::set_substitution(ir_type, std::move(structural_key));
  return true;
}

static bool try_build_type_ir(const TypePtr & type,
                              const TypeMangleContext * mangle_ctx,
                              abi_mangle::Type & out);

static bool try_build_typed_member_named_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base ||
     base->kind != Type::TK_NAMED ||
     !base->named_member_owner_type ||
     base->named_member_name.empty()) {
    return false;
  }

  abi_mangle::Type owner;
  if(!try_build_type_ir(base->named_member_owner_type, mangle_ctx, owner)) {
    return false;
  }

  abi_mangle::Type ir_type =
      abi_mangle::Type::member_named_type(std::move(owner),
                                          base->named_member_name,
                                          string());
  if(!attach_member_type_ir_substitution(ir_type, string())) {
    return false;
  }
  out = std::move(ir_type);
  return true;
}

static bool build_name_prefix_components_ir(
    const QualifiedName * prefix,
    vector<abi_mangle::Type::NameComponent> & prefix_components,
    string & canonical_prefix)
{
  prefix_components.clear();
  canonical_prefix.clear();
  if(!prefix || prefix->name.empty()) {
    return true;
  }
  if(prefix->rooted) {
    return false;
  }

  vector<string> prefix_parts = prefix->qualifiers;
  prefix_parts.push_back(prefix->name);
  for(size_t i = 0; i < prefix_parts.size(); ++i) {
    const string canonical_component = canonical_component_text(prefix_parts[i]);
    if(i == 0 && canonical_component == "std") {
      prefix_components.push_back(
          abi_mangle::Type::NameComponent::std_namespace());
      canonical_prefix = "std";
      continue;
    }
    const string full_name =
        append_qualified_component_text(canonical_prefix, canonical_component);
    prefix_components.push_back(
        abi_mangle::Type::NameComponent::source(prefix_parts[i],
                                                       full_name));
    canonical_prefix = full_name;
  }
  return true;
}

static bool try_build_template_entity_argument_name_ir(
    const TemplateArgument & argument,
    vector<abi_mangle::Type::NameComponent> & prefix_components,
    string & template_name,
    string & template_name_substitution,
    const TypeMangleContext * mangle_ctx = nullptr,
    abi_mangle::Type * template_owner = nullptr)
{
  if(template_owner) {
    *template_owner = abi_mangle::Type();
  }
  if(argument.dependent) {
    return false;
  }

  const semantic_model::Scope * declaring_scope = nullptr;
  template_name = trim_space(argument.template_entity_name);
  if(argument.kind == TemplateArgument::TA_CLASS_TEMPLATE) {
    if(argument.template_decl) {
      const semantic_model::ClassTemplateDecl * decl =
          static_cast<const semantic_model::ClassTemplateDecl *>(argument.template_decl);
      if(decl) {
        declaring_scope = decl->declaring_scope;
        if(template_name.empty()) {
          template_name = trim_space(decl->name);
        }
      }
    }
  } else if(argument.kind == TemplateArgument::TA_ALIAS_TEMPLATE) {
    if(argument.template_decl) {
      const semantic_model::AliasTemplateDecl * decl =
          static_cast<const semantic_model::AliasTemplateDecl *>(argument.template_decl);
      if(decl) {
        declaring_scope = decl->declaring_scope;
        if(template_name.empty()) {
          template_name = trim_space(decl->name);
        }
      }
    }
  } else {
    return false;
  }
  if(template_name.empty()) {
    return false;
  }

  if(argument.template_owner_type) {
    if(!template_owner ||
       !try_build_type_ir(argument.template_owner_type, mangle_ctx, *template_owner)) {
      return false;
    }
    template_name_substitution = !argument.text.empty() ?
        trim_space(argument.text) :
        string();
    if(template_name_substitution.empty()) {
      const string owner_text = selected_named_type_text(argument.template_owner_type);
      template_name_substitution = owner_text.empty() ?
          template_name :
          append_qualified_component_text(owner_text,
                                          canonical_component_text(template_name));
    }
    prefix_components.clear();
    return !template_name.empty();
  }

  QualifiedName prefix;
  const QualifiedName * prefix_ptr = nullptr;
  if(declaring_scope) {
    if(scope_prefix_syntax_for_template_decl(declaring_scope, prefix)) {
      prefix_ptr = &prefix;
    }
  } else if(!argument.template_entity_name_syntax.name.empty() &&
            argument.template_entity_name_syntax.name == template_name &&
            !argument.template_entity_name_syntax.qualifiers.empty()) {
    prefix.rooted = argument.template_entity_name_syntax.rooted;
    prefix.qualifiers = argument.template_entity_name_syntax.qualifiers;
    prefix.name = prefix.qualifiers.back();
    prefix.qualifiers.pop_back();
    prefix_ptr = &prefix;
  } else if(argument.source_syntax) {
    QualifiedName entity_name;
    const bool have_entity_name =
        replacement_syntax_qualified_name(*argument.source_syntax,
                                          entity_name,
                                          nullptr);
    if(have_entity_name &&
       entity_name.name == template_name &&
       !entity_name.qualifiers.empty()) {
      prefix.rooted = entity_name.rooted;
      prefix.qualifiers = entity_name.qualifiers;
      prefix.name = prefix.qualifiers.back();
      prefix.qualifiers.pop_back();
      prefix_ptr = &prefix;
    } else if(!trim_space(argument.template_entity_scope_prefix).empty()) {
      return false;
    }
  } else if(!trim_space(argument.template_entity_scope_prefix).empty()) {
    return false;
  }
  string canonical_prefix;
  if(!build_name_prefix_components_ir(prefix_ptr,
                                      prefix_components,
                                      canonical_prefix)) {
    return false;
  }
  template_name_substitution =
      append_qualified_component_text(canonical_prefix,
                                      canonical_component_text(template_name));
  return !template_name_substitution.empty();
}

static bool try_emit_static_member_object_symbol_ir(
    const semantic_model::ClassInfo & owner_class,
    const string & member_name,
    string & out,
    const vector<TemplateArgument> * template_arguments = nullptr,
    const vector<TemplateParameterInfo> * template_parameters = nullptr);

struct ExternalEntityArgumentIrPayload
{
  string symbol;
  bool address_of = false;
  bool has_member_semantics = false;
  abi_mangle::Type owner_type;
  string member_name;
  vector<abi_mangle::Type> parameter_types;
  bool is_function = false;
  bool function_const = false;
  bool function_volatile = false;
  bool function_lvalue_ref = false;
  bool function_rvalue_ref = false;
  bool function_variadic = false;
};

static string unqualified_external_member_name(const string & name)
{
  const size_t pos = name.rfind("::");
  if(pos == string::npos) {
    return name;
  }
  return trim_space(name.substr(pos + 2));
}

static bool build_external_member_function_payload(
    const semantic_model::FunctionBinding & binding,
    const TypeMangleContext * mangle_ctx,
    ExternalEntityArgumentIrPayload & payload)
{
  if(!binding.owner_class ||
     !binding.owner_class->type ||
     binding.name.empty()) {
    return false;
  }

  abi_mangle::Type owner;
  if(!try_build_type_ir(binding.owner_class->type, mangle_ctx, owner)) {
    return false;
  }

  TypePtr function_type = strip_top_level_cv(
      binding.declared_type ? binding.declared_type : binding.type);
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return false;
  }

  vector<abi_mangle::Type> params;
  params.reserve(function_type->params.size());
  for(size_t i = 0; i < function_type->params.size(); ++i) {
    abi_mangle::Type param;
    if(!try_build_type_ir(function_type->params[i], mangle_ctx, param)) {
      return false;
    }
    params.push_back(std::move(param));
  }

  payload.has_member_semantics = true;
  payload.owner_type = std::move(owner);
  payload.member_name = unqualified_external_member_name(binding.name);
  payload.parameter_types = std::move(params);
  payload.is_function = true;
  payload.function_const = binding.is_const_method || function_type->function_const;
  payload.function_volatile =
      binding.is_volatile_method || function_type->function_volatile;
  payload.function_lvalue_ref =
      binding.ref_qualifier == semantic_model::RQ_LVALUE;
  payload.function_rvalue_ref =
      binding.ref_qualifier == semantic_model::RQ_RVALUE;
  payload.function_variadic = function_type->variadic;
  return true;
}

static bool build_external_member_object_payload(
    const semantic_model::ValueBinding & binding,
    const TypeMangleContext * mangle_ctx,
    ExternalEntityArgumentIrPayload & payload)
{
  if(binding.kind != semantic_model::ValueBinding::VK_FIELD ||
     !binding.owner_class ||
     !binding.owner_class->type ||
     binding.name.empty()) {
    return false;
  }

  abi_mangle::Type owner;
  if(!try_build_type_ir(binding.owner_class->type, mangle_ctx, owner)) {
    return false;
  }

  payload.has_member_semantics = true;
  payload.owner_type = std::move(owner);
  payload.member_name = binding.name;
  payload.parameter_types.clear();
  payload.is_function = false;
  payload.function_const = false;
  payload.function_volatile = false;
  payload.function_lvalue_ref = false;
  payload.function_rvalue_ref = false;
  payload.function_variadic = false;
  return true;
}

static bool scope_value_binding_matches_argument(
    const semantic_model::ValueBinding & candidate,
    const semantic_model::ValueBinding * binding)
{
  if(!binding) {
    return false;
  }
  if(&candidate == binding) {
    return true;
  }
  return candidate.name == binding->name &&
         candidate.kind == binding->kind &&
         candidate.owner_class == binding->owner_class &&
         !candidate.symbol.object_symbol.empty() &&
         candidate.symbol.object_symbol == binding->symbol.object_symbol;
}

static const semantic_model::Scope * direct_scope_for_value_binding(
    const semantic_model::Scope * scope,
    const semantic_model::ValueBinding * binding)
{
  if(!scope || !binding) {
    return nullptr;
  }
  std::map<string, semantic_model::ValueBinding>::const_iterator found =
      scope->values.find(binding->name);
  if(found != scope->values.end() &&
     scope_value_binding_matches_argument(found->second, binding)) {
    return scope;
  }
  return nullptr;
}

static const semantic_model::Scope * find_value_binding_scope_in_namespace_tree(
    const semantic_model::Scope * scope,
    const semantic_model::ValueBinding * binding)
{
  if(!scope || !binding) {
    return nullptr;
  }
  if(const semantic_model::Scope * direct =
         direct_scope_for_value_binding(scope, binding)) {
    return direct;
  }
  for(size_t i = 0; i < scope->namespace_children.size(); ++i) {
    if(const semantic_model::Scope * found =
           find_value_binding_scope_in_namespace_tree(
               scope->namespace_children[i].get(),
               binding)) {
      return found;
    }
  }
  return nullptr;
}

static const semantic_model::Scope * find_value_binding_scope_for_symbol(
    const semantic_model::ValueBinding & binding,
    const TypeMangleContext * mangle_ctx)
{
  if(binding.declaration_scope) {
    return binding.declaration_scope;
  }
  if(!mangle_ctx || !mangle_ctx->lookup_scope) {
    return nullptr;
  }
  for(const semantic_model::Scope * current = mangle_ctx->lookup_scope;
      current;
      current = current->parent) {
    if(const semantic_model::Scope * direct =
           direct_scope_for_value_binding(current, &binding)) {
      return direct;
    }
  }
  return find_value_binding_scope_in_namespace_tree(
      root_scope(mangle_ctx->lookup_scope),
      &binding);
}

static bool try_build_value_binding_object_symbol_ir_payload(
    const semantic_model::ValueBinding & binding,
    const TypeMangleContext * mangle_ctx,
    string & out)
{
  out.clear();
  if(!binding.symbol.object_symbol.empty()) {
    out = binding.symbol.object_symbol;
    return true;
  }
  if(binding.kind != semantic_model::ValueBinding::VK_VARIABLE) {
    return false;
  }
  if(binding.owner_class) {
    return try_emit_static_member_object_symbol_ir(*binding.owner_class,
                                                   binding.name,
                                                   out);
  }
  const semantic_model::Scope * scope =
      find_value_binding_scope_for_symbol(binding, mangle_ctx);
  if(!scope) {
    return false;
  }
  try {
    SymbolIdentity symbol =
        make_scoped_variable_symbol_identity(*scope,
                                             binding.name,
                                             binding.is_c_linkage,
                                             binding.symbol.linkage);
    out = symbol.object_symbol;
    return !out.empty();
  } catch(const std::logic_error &) {
    out.clear();
    return false;
  }
}

static bool try_build_external_entity_argument_ir_payload(
    const TemplateArgument & argument,
    const TypeMangleContext * mangle_ctx,
    ExternalEntityArgumentIrPayload & payload)
{
  payload = ExternalEntityArgumentIrPayload();
  if(argument.dependent) {
    return false;
  }

  TypePtr base = strip_top_level_cv(argument.type);
  if(argument.function_value && base) {
    const bool is_function_pointer =
        base->kind == Type::TK_POINTER &&
        base->inner &&
        strip_top_level_cv(base->inner)->kind == Type::TK_FUNCTION;
    const bool is_function_reference =
        (base->kind == Type::TK_LVALUE_REFERENCE ||
         base->kind == Type::TK_RVALUE_REFERENCE) &&
        base->inner &&
        strip_top_level_cv(base->inner)->kind == Type::TK_FUNCTION;
    const bool is_member_function_pointer =
        base->kind == Type::TK_MEMBER_POINTER &&
        base->inner &&
        strip_top_level_cv(base->inner)->kind == Type::TK_FUNCTION;
    if(is_function_pointer || is_function_reference ||
       is_member_function_pointer) {
      payload.symbol = argument.function_value->symbol.object_symbol;
      payload.address_of = is_function_pointer || is_member_function_pointer;
      if(is_member_function_pointer) {
        return build_external_member_function_payload(*argument.function_value,
                                                      mangle_ctx,
                                                      payload);
      }
      return !payload.symbol.empty();
    }
  }

  if(argument.value_binding && base && base->kind == Type::TK_MEMBER_POINTER) {
    const semantic_model::ValueBinding * binding = argument.value_binding;
    if(binding->kind == semantic_model::ValueBinding::VK_FIELD) {
      payload.address_of = true;
      if(binding->owner_class) {
        try_emit_static_member_object_symbol_ir(*binding->owner_class,
                                                binding->name,
                                                payload.symbol);
      }
      return build_external_member_object_payload(*binding, mangle_ctx, payload);
    }
  }

  if(argument.value_binding && base) {
    const bool is_object_pointer =
        base->kind == Type::TK_POINTER &&
        base->inner &&
        strip_top_level_cv(base->inner)->kind != Type::TK_FUNCTION;
    const bool is_object_reference =
        base->kind == Type::TK_LVALUE_REFERENCE ||
        base->kind == Type::TK_RVALUE_REFERENCE;
    if(is_object_pointer) {
      payload.address_of = true;
      return try_build_value_binding_object_symbol_ir_payload(
          *argument.value_binding,
          mangle_ctx,
          payload.symbol);
    }
    if(is_object_reference) {
      payload.address_of = false;
      return try_build_value_binding_object_symbol_ir_payload(
          *argument.value_binding,
          mangle_ctx,
          payload.symbol);
    }
  }

  return false;
}

static abi_mangle::Type::ClassTemplateArgument
class_external_entity_argument_from_payload(
    const ExternalEntityArgumentIrPayload & payload)
{
  if(payload.has_member_semantics) {
    return abi_mangle::Type::ClassTemplateArgument::
        external_member_entity_arg(payload.symbol,
                                   payload.address_of,
                                   payload.owner_type,
                                   payload.member_name,
                                   payload.parameter_types,
                                   payload.is_function,
                                   payload.function_const,
                                   payload.function_volatile,
                                   payload.function_lvalue_ref,
                                   payload.function_rvalue_ref,
                                   payload.function_variadic);
  }
  return abi_mangle::Type::ClassTemplateArgument::external_entity_arg(
      payload.symbol,
      payload.address_of);
}

static abi_mangle::TemplateArgument
function_external_entity_argument_from_payload(
    const ExternalEntityArgumentIrPayload & payload)
{
  if(payload.has_member_semantics) {
    return abi_mangle::TemplateArgument::external_member_entity_arg(
        payload.symbol,
        payload.address_of,
        payload.owner_type,
        payload.member_name,
        payload.parameter_types,
        payload.is_function,
        payload.function_const,
        payload.function_volatile,
        payload.function_lvalue_ref,
        payload.function_rvalue_ref,
        payload.function_variadic);
  }
  return abi_mangle::TemplateArgument::external_entity_arg(
      payload.symbol,
      payload.address_of);
}

static bool try_build_context_free_type_ir(const TypePtr & type,
                                           const TypeMangleContext * mangle_ctx,
                                           abi_mangle::Type & out)
{
  if(!type) {
    return false;
  }

  switch(type->kind) {
  case Type::TK_FUNDAMENTAL: {
    string code;
    if(!append_fundamental_mangle_code(type->fundamental, code) || code.empty()) {
      return false;
    }
    out = abi_mangle::Type::builtin(code);
    return true;
  }

  case Type::TK_CV: {
    abi_mangle::Type inner;
    if(!try_build_context_free_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::cv(type->cv_const,
                               type->cv_volatile,
                               std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_POINTER: {
    abi_mangle::Type inner;
    if(!try_build_context_free_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::pointer(std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_LVALUE_REFERENCE: {
    abi_mangle::Type inner;
    if(!try_build_context_free_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::lvalue_reference(std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_RVALUE_REFERENCE: {
    abi_mangle::Type inner;
    if(!try_build_context_free_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::rvalue_reference(std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_ARRAY: {
    string bound;
    if(!build_array_bound_type_mangle(type, bound)) {
      return false;
    }
    string bound_key;
    if(!build_array_bound_substitution_key(type, mangle_ctx, bound_key)) {
      return false;
    }
    abi_mangle::Type inner;
    if(!try_build_context_free_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::array(std::move(bound),
                                  std::move(bound_key),
                                  std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_FUNCTION: {
    abi_mangle::Type result;
    if(!try_build_context_free_type_ir(type->inner, mangle_ctx, result)) {
      return false;
    }
    vector<abi_mangle::Type> params;
    params.reserve(type->params.size());
    for(size_t i = 0; i < type->params.size(); ++i) {
      abi_mangle::Type param;
      if(!try_build_context_free_type_ir(type->params[i], mangle_ctx, param)) {
        return false;
      }
      params.push_back(std::move(param));
    }
    out = abi_mangle::Type::function(
        std::move(result),
        std::move(params),
        type->variadic,
        type->function_ref_qualifier == FTRQ_LVALUE,
        type->function_ref_qualifier == FTRQ_RVALUE);
    apply_function_type_cv_ir(type, out);
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_ATOMIC: {
    abi_mangle::Type inner;
    if(!try_build_context_free_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::vendor_qualified("_Atomic", std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_NAMED:
  case Type::TK_MEMBER_POINTER:
  case Type::TK_BLOCK_POINTER:
    return false;
  }

  return false;
}

static bool type_has_contextual_local_mangle_state(const TypePtr & type)
{
  if(!type) {
    return false;
  }
  if(type->kind == Type::TK_NAMED &&
     (type->named_display.find("__local_") != string::npos ||
      type->named_key.find("__local_") != string::npos ||
      type->named_display.find("(anonymous namespace)") != string::npos ||
      type->named_key.find("(anonymous namespace)") != string::npos)) {
    return true;
  }
  if(type_has_contextual_local_mangle_state(type->owner) ||
     type_has_contextual_local_mangle_state(type->inner)) {
    return true;
  }
  for(size_t i = 0; i < type->params.size(); ++i) {
    if(type_has_contextual_local_mangle_state(type->params[i])) {
      return true;
    }
  }
  return false;
}

static const TypeMangleContext * template_argument_type_mangle_context(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx)
{
  if(mangle_ctx &&
     type &&
     !type_has_dependent_mangle_state(type) &&
     !type_has_contextual_local_mangle_state(type)) {
    return nullptr;
  }
  return mangle_ctx;
}

static bool try_build_template_argument_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  return try_build_type_ir(
      type,
      template_argument_type_mangle_context(type, mangle_ctx),
      out);
}

static bool try_build_type_id_ast_ir(const CppAstNode & node,
                                     const TypeMangleContext * mangle_ctx,
                                     abi_mangle::Type & out);
static bool try_build_type_specifier_seq_ast_ir(const CppAstNode & node,
                                                const TypeMangleContext * mangle_ctx,
                                                abi_mangle::Type & out);
static bool try_build_declarator_ast_type_ir(
    const CppAstNode & declarator,
    const function<bool(abi_mangle::Type &)> & build_base,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out);
static bool try_build_parameter_declaration_ast_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out);
static bool try_build_dependent_expression_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::DependentExpression & out);
static bool try_build_template_argument_syntax_ir(
    const TemplateArgumentSyntax & syntax,
    const TemplateParameterInfo * parameter,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type::ClassTemplateArgument & out);
static bool try_build_template_id_type_ir(const TemplateIdSyntax & syntax,
                                          const TypeMangleContext * mangle_ctx,
                                          abi_mangle::Type & out,
                                          bool expand_alias_templates = true,
                                          bool suppress_current_pack_grouping = false);
static bool try_build_direct_type_syntax_text_ir(
    const string & raw_text,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out,
    bool register_template_parameter_substitution = true);
static bool try_build_cast_target_type_id_ast_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out);
static bool try_build_class_template_argument_ir(
    const TemplateArgument & argument,
    const TemplateParameterInfo * parameter,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type::ClassTemplateArgument & out);
static bool try_emit_static_member_object_symbol_ir(
    const semantic_model::ClassInfo & owner_class,
    const string & member_name,
    string & out,
    const vector<TemplateArgument> * template_arguments,
    const vector<TemplateParameterInfo> * template_parameters);

static abi_mangle::SubstitutionKey template_parameter_substitution_key(
    size_t index,
    const vector<TemplateParameterInfo> * parameters)
{
  abi_mangle::SubstitutionKey key =
      abi_mangle::SubstitutionKey::type_template_parameter(index);
  const string scope_payload = template_parameter_scope_payload(parameters);
  if(!scope_payload.empty()) {
    key.payload = scope_payload;
  }
  return key;
}

static abi_mangle::Type template_parameter_type_ir(
    size_t index,
    const TemplateParameterInfo & parameter,
    const vector<TemplateParameterInfo> * parameters,
    bool register_substitution)
{
  (void)parameter;
  abi_mangle::Type out = abi_mangle::Type::template_parameter(index);
  if(register_substitution) {
    abi_mangle::set_substitution(
        out,
        template_parameter_substitution_key(index, parameters));
  }
  return out;
}

static void wrap_pack_expansion_type_ir_if_needed(
    bool pack_expansion,
    abi_mangle::Type & out,
    bool register_substitution = true)
{
  if(pack_expansion &&
     out.kind != abi_mangle::Type::TK_PACK_EXPANSION) {
    out = abi_mangle::Type::pack_expansion(std::move(out));
    if(register_substitution) {
      attach_context_free_type_ir_substitution(out);
    }
  }
}

static bool try_build_template_parameter_type_text_ir(
    const string & text,
    const TypeMangleContext * mangle_ctx,
    bool register_substitution,
    abi_mangle::Type & out)
{
  if(!mangle_ctx) {
    return false;
  }

  string stripped = trim_elaborated_type_prefix(text);
  bool pack_expansion = false;
  if(stripped.size() >= 3 &&
     stripped.compare(stripped.size() - 3, 3, "...") == 0) {
    stripped = trim_space(stripped.substr(0, stripped.size() - 3));
    pack_expansion = true;
  }
  if(stripped.empty()) {
    return false;
  }

  if(mangle_ctx->template_parameters &&
     mangle_ctx->template_parameters->parameters) {
    const vector<TemplateParameterInfo> * parameters =
        mangle_ctx->template_parameters->parameters;
    for(size_t i = 0; i < parameters->size(); ++i) {
      const TemplateParameterInfo & param = (*parameters)[i];
      if(param.kind != TemplateParameterInfo::TP_TYPE ||
         param.name.empty()) {
        continue;
      }
      if(text_matches_type_parameter_name(stripped, param.name) ||
         stripped == param.placeholder_key) {
        out = template_parameter_type_ir(i,
                                         param,
                                         parameters,
                                         register_substitution &&
                                         !mangle_ctx->suppress_template_parameter_type_registration);
        wrap_pack_expansion_type_ir_if_needed(pack_expansion,
                                              out,
                                              register_substitution);
        return true;
      }
    }
  }

  if(mangle_ctx->owner_template_parameters &&
     mangle_ctx->owner_template_arguments) {
    const vector<TemplateParameterInfo> & parameters =
        *mangle_ctx->owner_template_parameters;
    const vector<TemplateArgument> & arguments =
        *mangle_ctx->owner_template_arguments;
    for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
      const TemplateParameterInfo & param = parameters[i];
      if(param.kind != TemplateParameterInfo::TP_TYPE ||
         param.name.empty()) {
        continue;
      }
      if(!text_matches_type_parameter_name(stripped, param.name) &&
         stripped != param.placeholder_key) {
        continue;
      }
      if(owner_template_argument_index_is_suppressed(mangle_ctx, i) ||
         template_argument_is_self_type_parameter(
             arguments[i], param, TypePtr(), stripped)) {
        out = template_parameter_type_ir(i,
                                         param,
                                         &parameters,
                                         register_substitution);
        wrap_pack_expansion_type_ir_if_needed(pack_expansion,
                                              out,
                                              register_substitution);
        return true;
      }
      if(arguments[i].kind == TemplateArgument::TA_TYPE &&
         arguments[i].type) {
        TypeMangleContext owner_arg_ctx_storage;
        if(try_build_template_argument_type_ir(
               arguments[i].type,
               suppress_owner_template_argument_index(
                   mangle_ctx, i, owner_arg_ctx_storage),
               out)) {
          if(!(pack_expansion && param.parameter_pack)) {
            wrap_pack_expansion_type_ir_if_needed(pack_expansion,
                                                  out,
                                                  register_substitution);
          }
          return true;
        }
      }
    }
  }

  return false;
}

static bool try_build_template_parameter_type_ast_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    bool register_substitution,
    abi_mangle::Type & out)
{
  if((node.kind != CppAstKind::type_name &&
      node.kind != CppAstKind::decl_specifier) ||
     cppast_template_id_syntax(node)) {
    return false;
  }
  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  if(qualified &&
     (qualified->rooted ||
      !qualified->qualifiers.empty())) {
    return false;
  }
  const string name =
      qualified && !qualified->name.empty() ? qualified->name : node.value;
  return try_build_template_parameter_type_text_ir(name,
                                                   mangle_ctx,
                                                   register_substitution,
                                                   out);
}

static string strip_named_semantic_match_prefix(const string & text)
{
  string stripped = trim_elaborated_type_prefix(text);
  static const char * prefixes[] = {
      "template-parameter ",
      "partial-order ",
      "dependent type ",
      "dependent alias ",
      "dependent decltype ",
      "dependent typeof "
  };
  for(size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const string prefix = prefixes[i];
    if(stripped.compare(0, prefix.size(), prefix) == 0) {
      return trim_space(stripped.substr(prefix.size()));
    }
  }
  return stripped;
}

static bool try_build_template_parameter_type_spelling_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(!type || type->kind != Type::TK_NAMED) {
    return false;
  }

  const bool register_substitution =
      !mangle_ctx || !mangle_ctx->suppress_template_parameter_type_registration;
  vector<string> candidates;
  candidates.push_back(type->named_semantic_payload);
  candidates.push_back(type->named_display);
  candidates.push_back(type->named_key);
  for(size_t i = 0; i < candidates.size(); ++i) {
    const string stripped = strip_named_semantic_match_prefix(candidates[i]);
    if(stripped.empty()) {
      continue;
    }
    if(try_build_template_parameter_type_text_ir(stripped,
                                                 mangle_ctx,
                                                 register_substitution,
                                                 out)) {
      return true;
    }
  }
  return false;
}

static bool try_build_template_parameter_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(!mangle_ctx ||
     !type ||
     type->kind != Type::TK_NAMED) {
    return false;
  }
  if(type->named_semantic_kind != Type::NSK_TEMPLATE_PARAMETER) {
    return try_build_template_parameter_type_spelling_ir(type, mangle_ctx, out);
  }

  const string payload =
      trim_elaborated_type_prefix(type->named_semantic_payload);
  if(payload.empty()) {
    return try_build_template_parameter_type_spelling_ir(type, mangle_ctx, out);
  }
  const auto placeholder_payload_matches =
      [&payload](const string & placeholder_key) -> bool
      {
        if(placeholder_key.empty()) {
          return false;
        }
        static const char template_parameter_prefix[] =
            "template-parameter ";
        if(placeholder_key.compare(0,
                                   sizeof(template_parameter_prefix) - 1,
                                   template_parameter_prefix) == 0) {
          return payload ==
              placeholder_key.substr(sizeof(template_parameter_prefix) - 1);
        }
        return payload == placeholder_key;
      };

  if(mangle_ctx->template_parameters &&
     mangle_ctx->template_parameters->parameters) {
    const vector<TemplateParameterInfo> * parameters =
        mangle_ctx->template_parameters->parameters;
    for(size_t i = 0; i < parameters->size(); ++i) {
      const TemplateParameterInfo & param = (*parameters)[i];
      if(param.kind != TemplateParameterInfo::TP_TYPE) {
        continue;
      }
      if((!param.name.empty() &&
          text_matches_type_parameter_name(payload, param.name)) ||
         placeholder_payload_matches(param.placeholder_key)) {
        out = template_parameter_type_ir(i,
                                         param,
                                         parameters,
                                         !mangle_ctx->suppress_template_parameter_type_registration);
        return true;
      }
    }
  }

  if(mangle_ctx->owner_template_parameters &&
     mangle_ctx->owner_template_arguments) {
    const vector<TemplateParameterInfo> & parameters =
        *mangle_ctx->owner_template_parameters;
    const vector<TemplateArgument> & arguments =
        *mangle_ctx->owner_template_arguments;
    for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
      const TemplateParameterInfo & param = parameters[i];
      if(param.kind != TemplateParameterInfo::TP_TYPE ||
         ((param.name.empty() ||
           !text_matches_type_parameter_name(payload, param.name)) &&
          !placeholder_payload_matches(param.placeholder_key))) {
        continue;
      }
      if(owner_template_argument_index_is_suppressed(mangle_ctx, i) ||
         template_argument_is_self_type_parameter(
             arguments[i], param, type, payload)) {
        out = template_parameter_type_ir(i, param, &parameters, true);
        return true;
      }
      if(arguments[i].kind == TemplateArgument::TA_TYPE &&
         arguments[i].type) {
        TypeMangleContext owner_arg_ctx_storage;
        return try_build_template_argument_type_ir(
            arguments[i].type,
            suppress_owner_template_argument_index(
                mangle_ctx, i, owner_arg_ctx_storage),
            out);
      }
    }
  }

  return try_build_template_parameter_type_spelling_ir(type, mangle_ctx, out);
}

static bool type_mentions_owner_type_template_parameter_pack(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx)
{
  if(!type ||
     !mangle_ctx ||
     !mangle_ctx->owner_template_parameters) {
    return false;
  }
  if(type->kind == Type::TK_NAMED &&
     type->named_semantic_kind == Type::NSK_TEMPLATE_PARAMETER) {
    const string payload =
        strip_named_semantic_match_prefix(type->named_semantic_payload);
    const string display =
        strip_named_semantic_match_prefix(type->named_display);
    const string key =
        strip_named_semantic_match_prefix(type->named_key);
    const vector<TemplateParameterInfo> & parameters =
        *mangle_ctx->owner_template_parameters;
    for(size_t i = 0; i < parameters.size(); ++i) {
      const TemplateParameterInfo & parameter = parameters[i];
      if(parameter.kind != TemplateParameterInfo::TP_TYPE ||
         !parameter.parameter_pack ||
         parameter.name.empty()) {
        continue;
      }
      if(text_matches_type_parameter_name(payload, parameter.name) ||
         text_matches_type_parameter_name(display, parameter.name) ||
         text_matches_type_parameter_name(key, parameter.name) ||
         (!parameter.placeholder_key.empty() &&
          (payload == parameter.placeholder_key ||
           display == parameter.placeholder_key ||
           key == parameter.placeholder_key))) {
        return true;
      }
    }
  }
  if(type_mentions_owner_type_template_parameter_pack(type->owner, mangle_ctx) ||
     type_mentions_owner_type_template_parameter_pack(type->inner, mangle_ctx)) {
    return true;
  }
  for(size_t i = 0; i < type->params.size(); ++i) {
    if(type_mentions_owner_type_template_parameter_pack(type->params[i],
                                                        mangle_ctx)) {
      return true;
    }
  }
  return false;
}

static bool try_build_unbound_template_parameter_type_ir(
    const TypePtr & type,
    abi_mangle::Type & out)
{
  if(!type ||
     type->kind != Type::TK_NAMED ||
     type->named_semantic_kind != Type::NSK_TEMPLATE_PARAMETER) {
    return false;
  }

  string source_name = canonical_template_argument_text(type->named_display);
  if(source_name.empty()) {
    source_name = canonical_template_argument_text(type->named_key);
  }
  if(source_name.empty() ||
     source_name.find("::") != string::npos ||
     !is_identifier_text_for_mangling(source_name)) {
    return false;
  }

  const string canonical_source_name = canonical_component_text(source_name);
  out = abi_mangle::Type::named_type(
      vector<abi_mangle::Type::NameComponent>(),
      std::move(source_name),
      canonical_source_name);
  abi_mangle::set_substitution(
      out,
      abi_mangle::SubstitutionKey::named(canonical_source_name));
  return true;
}

static bool try_build_template_parameter_value_expression_ir(
    const string & text,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::DependentExpression & out)
{
  if(!mangle_ctx) {
    return false;
  }
  const string stripped = trim_space(text);
  const auto try_parameters =
      [&](const vector<TemplateParameterInfo> * parameters) -> bool
      {
        if(!parameters) {
          return false;
        }
        for(size_t i = 0; i < parameters->size(); ++i) {
          const TemplateParameterInfo & parameter = (*parameters)[i];
          if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE &&
             ((!parameter.name.empty() && stripped == parameter.name) ||
              (!parameter.placeholder_key.empty() &&
               stripped == parameter.placeholder_key))) {
            out = abi_mangle::DependentExpression::template_parameter(i);
            return true;
          }
          for(size_t j = 0; j < parameter.alternate_names.size(); ++j) {
            if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE &&
               stripped == parameter.alternate_names[j]) {
              out = abi_mangle::DependentExpression::template_parameter(i);
              return true;
            }
          }
        }
        return false;
      };
  if(mangle_ctx->template_parameters &&
     try_parameters(mangle_ctx->template_parameters->parameters)) {
    return true;
  }
  return try_parameters(mangle_ctx->owner_template_parameters);
}

static bool try_build_owner_non_type_template_argument_ir(
    const string & text,
    bool pack_expansion,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type::ClassTemplateArgument & out)
{
  if(!mangle_ctx ||
     !mangle_ctx->owner_template_parameters ||
     !mangle_ctx->owner_template_arguments) {
    return false;
  }

  const string stripped = trim_elaborated_type_prefix(text);
  if(stripped.empty()) {
    return false;
  }

  const vector<TemplateParameterInfo> & parameters =
      *mangle_ctx->owner_template_parameters;
  const vector<TemplateArgument> & arguments =
      *mangle_ctx->owner_template_arguments;
  for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
    const TemplateParameterInfo & parameter = parameters[i];
    const TemplateArgument & argument = arguments[i];
    const bool matches_parameter =
        template_parameter_identifier_matches(parameter, stripped);
    const bool matches_dependent_self_argument =
        argument.kind == TemplateArgument::TA_VALUE &&
        argument.dependent &&
        trim_elaborated_type_prefix(argument.text) == stripped;
    if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE ||
       (!matches_parameter && !matches_dependent_self_argument)) {
      continue;
    }

    if(owner_template_argument_index_is_suppressed(mangle_ctx, i) ||
       matches_dependent_self_argument) {
      abi_mangle::DependentExpression expression =
          abi_mangle::DependentExpression::template_parameter(i);
      if(pack_expansion) {
        expression =
            abi_mangle::DependentExpression::pack_expansion(expression);
      }
      out =
          abi_mangle::Type::ClassTemplateArgument::dependent_expression_arg(
              expression);
      return true;
    }

    TypeMangleContext owner_arg_ctx_storage;
    if(!try_build_class_template_argument_ir(
           argument,
           &parameter,
           suppress_owner_template_argument_index(mangle_ctx,
                                                  i,
                                                  owner_arg_ctx_storage),
           out)) {
      return false;
    }
    return !pack_expansion;
  }

  return false;
}

static string non_type_template_argument_expression_text_for_ir(
    const TemplateArgumentSyntax & syntax)
{
  string text = trim_space(syntax.text);
  if(syntax.pack_expansion &&
     text.size() >= 3 &&
     text.compare(text.size() - 3, 3, "...") == 0) {
    text = trim_space(text.substr(0, text.size() - 3));
  }
  return text;
}

static bool try_build_sizeof_pack_expression_ir(
    const string & text,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::DependentExpression & out)
{
  if(!mangle_ctx) {
    return false;
  }

  const string stripped = trim_elaborated_type_prefix(text);
  const auto try_parameters =
      [&](const vector<TemplateParameterInfo> * parameters) -> bool
      {
        if(!parameters) {
          return false;
        }
        for(size_t i = 0; i < parameters->size(); ++i) {
          const TemplateParameterInfo & parameter = (*parameters)[i];
          if(!parameter.parameter_pack ||
             parameter.name.empty() ||
             stripped != parameter.name) {
            continue;
          }
          abi_mangle::DependentExpression parameter_expr =
              abi_mangle::DependentExpression::template_parameter(i);
          out = abi_mangle::DependentExpression::unary("sZ",
                                                              parameter_expr);
          return true;
        }
        return false;
      };

  if(mangle_ctx->template_parameters &&
     try_parameters(mangle_ctx->template_parameters->parameters)) {
    return true;
  }
  return try_parameters(mangle_ctx->owner_template_parameters);
}

static void prepend_qualified_name_qualifiers(vector<string> qualifiers,
                                              QualifiedName & name);

static bool ast_contains_identifier_value(const CppAstNode & node,
                                          const string & name)
{
  if(node.kind == CppAstKind::identifier && node.value == name) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_contains_identifier_value(node.children[i], name)) {
      return true;
    }
  }
  return false;
}

static bool try_build_static_member_external_expression_from_template_syntax(
    const TemplateIdSyntax & qualifier_template_id,
    const string & member_name,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::DependentExpression & out)
{
  if(member_name.empty()) {
    return false;
  }

  const semantic_model::ClassTemplateDecl * class_template =
      lookup_class_template_for_template_id_syntax(qualifier_template_id,
                                                   mangle_ctx);
  if(class_template &&
     class_template->class_node &&
     !ast_contains_identifier_value(*class_template->class_node, member_name)) {
    return false;
  }

  abi_mangle::Type owner;
  if(!try_build_template_id_type_ir(qualifier_template_id,
                                    mangle_ctx,
                                    owner,
                                    false,
                                    true)) {
    return false;
  }

  string owner_mangle;
  MangleSubstitutionState local_state;
  MangleIrSubstitutionSink sink(&local_state);
  if(!abi_mangle::emit_type(owner, owner_mangle, &sink) ||
     owner_mangle.empty()) {
    return false;
  }

  string symbol;
  if(!abi_mangle::emit_external_member_entity_symbol(&owner,
                                                     member_name,
                                                     vector<abi_mangle::Type>(),
                                                     false,
                                                     false,
                                                     false,
                                                     false,
                                                     false,
                                                     false,
                                                     symbol,
                                                     &sink)) {
    return false;
  }
  out = abi_mangle::DependentExpression::external_entity(symbol,
                                                                false);
  return true;
}

static bool try_build_resolved_constant_member_expression_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::DependentExpression & out)
{
  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  if(!qualified || qualified->qualifiers.empty() || qualified->name.empty() ||
     node.qualifier_template_id_syntaxes.empty()) {
    return false;
  }

  TemplateIdSyntax qualifier_template_id;
  size_t template_qualifier_index = string::npos;
  for(size_t i = qualified->qualifiers.size();
      i > 0 && i <= node.qualifier_template_id_syntaxes.size();
      --i) {
    const TemplateIdSyntax & candidate =
        node.qualifier_template_id_syntaxes[i - 1];
    if(!candidate.name.name.empty()) {
      qualifier_template_id = candidate;
      template_qualifier_index = i - 1;
      break;
    }
  }
  if(template_qualifier_index == string::npos ||
     template_qualifier_index + 1 != qualified->qualifiers.size()) {
    return false;
  }

  vector<string> leading_qualifiers(qualified->qualifiers.begin(),
                                    qualified->qualifiers.begin() +
                                    template_qualifier_index);
  if(qualifier_template_id.name.qualifiers.empty()) {
    prepend_qualified_name_qualifiers(leading_qualifiers,
                                      qualifier_template_id.name);
  }

  if(template_id_syntax_mentions_template_parameter(qualifier_template_id,
                                                    mangle_ctx)) {
    return false;
  }

  const semantic_model::ClassInfo * info =
      resolved_class_template_instantiation(qualifier_template_id, mangle_ctx);
  if(!info || !info->member_scope) {
    if(try_build_static_member_external_expression_from_template_syntax(
           qualifier_template_id,
           qualified->name,
           mangle_ctx,
           out)) {
      return true;
    }
    return false;
  }

  map<string, semantic_model::ValueBinding>::const_iterator found =
      info->member_scope->values.find(qualified->name);
  if(found == info->member_scope->values.end()) {
    return false;
  }

  if(!found->second.symbol.object_symbol.empty()) {
    out = abi_mangle::DependentExpression::external_entity(
        found->second.symbol.object_symbol,
        false);
    return true;
  }

  if(found->second.owner_class &&
     !(found->second.declaration_node &&
       found->second.declaration_node->kind == CppAstKind::enumerator)) {
    string symbol;
    if(try_emit_static_member_object_symbol_ir(*info,
                                                    qualified->name,
                                                    symbol)) {
      out = abi_mangle::DependentExpression::external_entity(symbol,
                                                                    false);
      return true;
    }
  }

  if(!found->second.has_constant_value || !found->second.type) {
    return false;
  }

  TypeMangleContext literal_type_ctx_storage;
  const TypeMangleContext * literal_type_ctx = mangle_ctx;
  if(mangle_ctx) {
    literal_type_ctx_storage = *mangle_ctx;
    literal_type_ctx_storage.suppress_expression_qualifier_template_name_substitution = true;
    literal_type_ctx = &literal_type_ctx_storage;
  }
  abi_mangle::Type value_type;
  if(!try_build_type_ir(found->second.type, literal_type_ctx, value_type)) {
    return false;
  }
  out = abi_mangle::DependentExpression::typed_integral_value(
      value_type,
      found->second.constant_value);
  return true;
}

static void prepend_qualified_name_qualifiers(vector<string> qualifiers,
                                              QualifiedName & name)
{
  if(qualifiers.empty()) {
    return;
  }
  qualifiers.insert(qualifiers.end(),
                    name.qualifiers.begin(),
                    name.qualifiers.end());
  name.qualifiers.swap(qualifiers);
}

static bool type_template_argument_requires_syntax_ir(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  switch(base->kind) {
  case Type::TK_NAMED:
    return base->named_semantic_kind != Type::NSK_TEMPLATE_PARAMETER &&
           (named_type_has_dependent_semantic(base) ||
            named_type_key_contains_dependent_semantic(base));

  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    return type_template_argument_requires_syntax_ir(base->inner);

  case Type::TK_MEMBER_POINTER:
    return type_template_argument_requires_syntax_ir(base->owner) ||
           type_template_argument_requires_syntax_ir(base->inner);

  case Type::TK_FUNCTION:
    if(type_template_argument_requires_syntax_ir(base->inner)) {
      return true;
    }
    for(size_t i = 0; i < base->params.size(); ++i) {
      if(type_template_argument_requires_syntax_ir(base->params[i])) {
        return true;
      }
    }
    return false;

  case Type::TK_FUNDAMENTAL:
    return false;
  }
  return false;
}

static bool try_build_dependent_type_template_argument_ir(
    const DependentAliasTemplateArgumentSyntax & argument,
    const TemplateParameterInfo & parameter,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type::ClassTemplateArgument & out)
{
  if(parameter.kind != TemplateParameterInfo::TP_TYPE ||
     !argument.type ||
     argument.source_defaulted ||
     (type_has_dependent_mangle_state(argument.type) &&
      type_template_argument_requires_syntax_ir(argument.type))) {
    return false;
  }

  abi_mangle::Type type;
  if(!try_build_template_argument_type_ir(argument.type, mangle_ctx, type)) {
    return false;
  }
  wrap_pack_expansion_type_ir_if_needed(argument.syntax.pack_expansion, type);
  out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
  return true;
}

static bool build_dependent_template_arguments_ir(
    const vector<DependentAliasTemplateArgumentSyntax> & arguments,
    const vector<TemplateParameterInfo> * parameters,
    const TypeMangleContext * mangle_ctx,
    vector<abi_mangle::Type::ClassTemplateArgument> & out)
{
  out.clear();
  if(parameters && !parameters->empty()) {
    size_t arg_index = 0;
    for(size_t i = 0; i < parameters->size(); ++i) {
      const TemplateParameterInfo & parameter = (*parameters)[i];
      if(parameter.parameter_pack) {
        size_t trailing_nonpack = 0;
        for(size_t j = i + 1; j < parameters->size(); ++j) {
          if(!(*parameters)[j].parameter_pack) {
            ++trailing_nonpack;
          }
        }
        if(arguments.size() < arg_index + trailing_nonpack) {
          return false;
        }
        const size_t pack_count =
            arguments.size() - arg_index - trailing_nonpack;
        vector<abi_mangle::Type::ClassTemplateArgument> pack_arguments;
        pack_arguments.reserve(pack_count);
        for(size_t j = 0; j < pack_count; ++j) {
          const DependentAliasTemplateArgumentSyntax & dependent_argument =
              arguments[arg_index++];
          abi_mangle::Type::ClassTemplateArgument ir_argument;
          if(!try_build_dependent_type_template_argument_ir(dependent_argument,
                                                            parameter,
                                                            mangle_ctx,
                                                            ir_argument) &&
             !try_build_template_argument_syntax_ir(
                 effective_dependent_argument_syntax(dependent_argument),
                 &parameter,
                 mangle_ctx,
                 ir_argument)) {
            if(parser_trace::enabled("symbol.linkage")) {
              ostringstream trace;
              trace << "dependent-template-argument-ir failed"
                    << " parameter=" << parameter.name
                    << " kind=" << static_cast<int>(parameter.kind)
                    << " pack=yes"
                    << " text=" << dependent_argument.text;
              parser_trace::note("symbol.linkage", string(), trace.str());
            }
            return false;
          }
          pack_arguments.push_back(std::move(ir_argument));
        }
        out.push_back(
            abi_mangle::Type::ClassTemplateArgument::argument_pack(
                std::move(pack_arguments)));
        continue;
      }
      if(arg_index >= arguments.size()) {
        return false;
      }
      const DependentAliasTemplateArgumentSyntax & dependent_argument =
          arguments[arg_index++];
      abi_mangle::Type::ClassTemplateArgument ir_argument;
      if(!try_build_dependent_type_template_argument_ir(dependent_argument,
                                                        parameter,
                                                        mangle_ctx,
                                                        ir_argument) &&
         !try_build_template_argument_syntax_ir(
             effective_dependent_argument_syntax(dependent_argument),
             &parameter,
             mangle_ctx,
             ir_argument)) {
        if(parser_trace::enabled("symbol.linkage")) {
          ostringstream trace;
          trace << "dependent-template-argument-ir failed"
                << " parameter=" << parameter.name
                << " kind=" << static_cast<int>(parameter.kind)
                << " pack=no"
                << " text=" << dependent_argument.text;
          parser_trace::note("symbol.linkage", string(), trace.str());
        }
        return false;
      }
      out.push_back(std::move(ir_argument));
    }
    return arg_index == arguments.size();
  }

  out.reserve(arguments.size());
  for(size_t i = 0; i < arguments.size(); ++i) {
    abi_mangle::Type::ClassTemplateArgument ir_argument;
    if(!try_build_template_argument_syntax_ir(
           effective_dependent_argument_syntax(arguments[i]),
           nullptr,
           mangle_ctx,
           ir_argument)) {
      if(parser_trace::enabled("symbol.linkage")) {
        ostringstream trace;
        trace << "dependent-template-argument-ir failed"
              << " parameter=<none>"
              << " pack=no"
              << " text=" << arguments[i].text;
        parser_trace::note("symbol.linkage", string(), trace.str());
      }
      return false;
    }
    out.push_back(std::move(ir_argument));
  }
  return true;
}

static const vector<TemplateParameterInfo> *
template_id_parameters_for_ir(const TemplateIdSyntax & syntax,
                              const vector<TemplateParameterInfo> * parameters,
                              bool suppress_current_pack_grouping,
                              vector<TemplateParameterInfo> & explicit_parameter_storage)
{
  if(!suppress_current_pack_grouping || !parameters) {
    return parameters;
  }
  if(syntax.argument_syntaxes.size() > parameters->size()) {
    return nullptr;
  }
  for(size_t i = 0; i < parameters->size(); ++i) {
    if((*parameters)[i].parameter_pack) {
      return nullptr;
    }
  }
  size_t parameter_count = syntax.argument_syntaxes.size();
  while(parameter_count < parameters->size()) {
    const TemplateParameterInfo & parameter = (*parameters)[parameter_count];
    if(!parameter.default_argument) {
      break;
    }
    ++parameter_count;
  }
  explicit_parameter_storage.assign(
      parameters->begin(),
      parameters->begin() + parameter_count);
  return &explicit_parameter_storage;
}

static bool class_template_argument_ir_to_template_argument_ir(
    const abi_mangle::Type::ClassTemplateArgument & in,
    abi_mangle::TemplateArgument & out)
{
  switch(in.kind) {
  case abi_mangle::Type::ClassTemplateArgument::CTAK_TYPE:
    if(!in.type) {
      return false;
    }
    out = abi_mangle::TemplateArgument::type_arg(*in.type);
    return true;

  case abi_mangle::Type::ClassTemplateArgument::CTAK_INTEGRAL_VALUE:
    if(!in.type) {
      return false;
    }
    out = abi_mangle::TemplateArgument::integral_value_arg(
        *in.type,
        in.integral_value);
    return true;

  case abi_mangle::Type::ClassTemplateArgument::CTAK_DEPENDENT_INTEGRAL_VALUE:
    if(!in.parameter_type) {
      return false;
    }
    if(in.type) {
      out = abi_mangle::TemplateArgument::dependent_integral_value_arg(
          *in.parameter_type,
          *in.type,
          in.integral_value);
    } else {
      out = abi_mangle::TemplateArgument::dependent_untyped_integral_value_arg(
          *in.parameter_type,
          in.integral_value);
    }
    return true;

  case abi_mangle::Type::ClassTemplateArgument::CTAK_DEPENDENT_EXPRESSION:
    if(!in.expression) {
      return false;
    }
    out = abi_mangle::TemplateArgument::dependent_expression_arg(
        *in.expression);
    return true;

  case abi_mangle::Type::ClassTemplateArgument::CTAK_UNTYPED_INTEGRAL_VALUE:
    out = abi_mangle::TemplateArgument::untyped_integral_value_arg(
        in.integral_value);
    return true;

  case abi_mangle::Type::ClassTemplateArgument::CTAK_TEMPLATE_ENTITY:
    if(!in.metadata) {
      return false;
    }
    if(in.metadata->template_name_is_template_parameter) {
      out = abi_mangle::TemplateArgument::
          template_parameter_template_arg(in.metadata->template_parameter_index);
      return true;
    }
    if(in.metadata->template_owner_type) {
      out = abi_mangle::TemplateArgument::member_template_entity_arg(
          *in.metadata->template_owner_type,
          in.metadata->template_name,
          in.metadata->template_name_substitution);
      return true;
    }
    out = abi_mangle::TemplateArgument::template_entity_arg(
        in.metadata->prefix_components,
        in.metadata->template_name,
        in.metadata->template_name_substitution);
    return true;

  case abi_mangle::Type::ClassTemplateArgument::CTAK_EXTERNAL_ENTITY:
    if(!in.metadata) {
      return false;
    }
    out = abi_mangle::TemplateArgument::external_entity_arg(
        in.metadata->external_entity_symbol,
        in.metadata->external_entity_address_of);
    return true;

  case abi_mangle::Type::ClassTemplateArgument::CTAK_ARGUMENT_PACK: {
    if(!in.metadata) {
      return false;
    }
    vector<abi_mangle::TemplateArgument> pack_arguments;
    pack_arguments.reserve(in.metadata->pack_arguments.size());
    for(size_t i = 0; i < in.metadata->pack_arguments.size(); ++i) {
      abi_mangle::TemplateArgument argument;
      if(!class_template_argument_ir_to_template_argument_ir(
             in.metadata->pack_arguments[i],
             argument)) {
        return false;
      }
      pack_arguments.push_back(std::move(argument));
    }
    out = abi_mangle::TemplateArgument::argument_pack(std::move(pack_arguments));
    return true;
  }

  case abi_mangle::Type::ClassTemplateArgument::CTAK_INVALID:
    return false;
  }
  return false;
}

static bool try_build_template_id_type_ir(const TemplateIdSyntax & syntax,
                                          const TypeMangleContext * mangle_ctx,
                                          abi_mangle::Type & out,
                                          bool expand_alias_templates,
                                          bool suppress_current_pack_grouping)
{
  CppAstNode alias_expansion;
  if(expand_alias_templates &&
     try_build_alias_template_id_expansion_syntax(syntax,
                                                  mangle_ctx,
                                                  alias_expansion)) {
    TypeMangleContext alias_ctx_storage;
    const TypeMangleContext * alias_ctx = mangle_ctx;
    if(mangle_ctx) {
      alias_ctx_storage = *mangle_ctx;
      ++alias_ctx_storage.alias_expansion_depth;
      alias_ctx_storage.suppress_current_type_id_substitution_registration = true;
      alias_ctx = &alias_ctx_storage;
    }
    return try_build_type_id_ast_ir(alias_expansion, alias_ctx, out);
  }

  const semantic_model::ClassTemplateDecl * class_template =
      lookup_class_template_for_template_id_syntax(syntax, mangle_ctx);
  const string base_name =
      strip_leading_template_disambiguator(syntax.name.name);
  if(base_name.empty()) {
    return false;
  }
  size_t template_template_parameter_index = 0;
  const TemplateParameterInfo * template_template_parameter = nullptr;
  const bool template_name_is_template_template_parameter =
      !syntax.name.rooted &&
      syntax.name.qualifiers.empty() &&
      try_find_template_template_parameter_index(
          base_name,
          mangle_ctx,
          template_template_parameter_index,
          template_template_parameter);

  const semantic_model::ClassInfo * member_owner_template_scope =
      class_template ? nearest_member_class_scope(class_template->declaring_scope) :
                       nullptr;
  shared_ptr<const ClassTemplateSpecializationMangleInfo> member_owner_info =
      member_owner_template_scope && member_owner_template_scope->type ?
          named_type_class_template_specialization_mangle_info_const(
              member_owner_template_scope->type) :
          shared_ptr<const ClassTemplateSpecializationMangleInfo>();
  const bool member_template_scope =
      member_owner_template_scope &&
      member_owner_template_scope->type &&
      member_owner_info;

  vector<abi_mangle::Type::NameComponent> prefix_components;
  string canonical_prefix;
  const bool preserve_source_template_prefix =
      mangle_ctx &&
      mangle_ctx->prefer_source_template_name_prefixes_in_expressions &&
      !syntax.name.rooted &&
      syntax.name.qualifiers.empty();
  if(class_template && !member_template_scope && !preserve_source_template_prefix) {
    QualifiedName prefix;
    const QualifiedName * prefix_ptr =
        scope_prefix_syntax_for_template_decl(class_template->declaring_scope,
                                              prefix) ?
            &prefix :
            nullptr;
    if(!build_name_prefix_components_ir(prefix_ptr,
                                        prefix_components,
                                        canonical_prefix)) {
      return false;
    }
  } else {
    QualifiedName prefix_name;
    prefix_name.rooted = syntax.name.rooted;
    prefix_name.qualifiers = syntax.name.qualifiers;
    if(syntax.name.qualifiers.empty()) {
      canonical_prefix.clear();
    } else {
      prefix_name.name = prefix_name.qualifiers.back();
      prefix_name.qualifiers.pop_back();
      if(!build_name_prefix_components_ir(
                  &prefix_name,
                  prefix_components,
                  canonical_prefix)) {
        return false;
      }
    }
  }

  vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
  dependent_arguments.reserve(syntax.argument_syntaxes.size());
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    DependentAliasTemplateArgumentSyntax argument;
    argument.syntax = syntax.argument_syntaxes[i];
    argument.text = argument.syntax.text;
    if(argument.text.empty() && i < syntax.arguments.size()) {
      argument.text = syntax.arguments[i];
      argument.syntax.text = argument.text;
    }
    if(argument.syntax.type_id && argument.syntax.type_id->semantic_type) {
      argument.type = argument.syntax.type_id->semantic_type;
    } else if(argument.syntax.resolved_type) {
      argument.type = argument.syntax.resolved_type;
    }
    dependent_arguments.push_back(std::move(argument));
  }

  if(template_name_is_template_template_parameter &&
     template_template_parameter) {
    vector<TemplateParameterInfo> template_template_parameters;
    const vector<TemplateParameterInfo> * argument_parameters = nullptr;
    if(template_template_parameter->template_parameter_count != 0 &&
       template_template_parameter->template_parameter_count != static_cast<size_t>(-1)) {
      template_template_parameters.resize(
          template_template_parameter->template_parameter_count);
      argument_parameters = &template_template_parameters;
    }

    vector<abi_mangle::Type::ClassTemplateArgument> arguments;
    if(!build_dependent_template_arguments_ir(dependent_arguments,
                                              argument_parameters,
                                              mangle_ctx,
                                              arguments)) {
      return false;
    }
	    abi_mangle::Type ir_type =
	        abi_mangle::Type::
	            template_parameter_class_template_specialization(
	                template_template_parameter_index,
	                std::move(arguments));
    out = std::move(ir_type);
    return true;
  }

  const vector<TemplateParameterInfo> * raw_parameters =
      class_template ? &class_template->parameters :
                       lookup_template_parameters_for_template_id_syntax(
                           syntax, mangle_ctx);
  if(raw_parameters &&
     dependent_arguments.size() < raw_parameters->size()) {
    dependent_arguments =
        complete_dependent_alias_template_arguments_for_mangling(
            *raw_parameters,
            dependent_arguments,
            template_id_default_argument_scope_for_mangling(syntax,
                                                            mangle_ctx));
  }
  vector<TemplateParameterInfo> explicit_parameter_storage;
  const vector<TemplateParameterInfo> * parameters =
      template_id_parameters_for_ir(syntax,
                                    raw_parameters,
                                    suppress_current_pack_grouping,
                                    explicit_parameter_storage);
  vector<abi_mangle::Type::ClassTemplateArgument> arguments;
  TypeMangleContext argument_ctx_storage;
  const TypeMangleContext * argument_ctx = mangle_ctx;
  if(mangle_ctx &&
     (mangle_ctx->prefer_source_template_name_prefixes_in_expressions ||
      (member_owner_info &&
       !member_owner_info->mangle_parameters.empty() &&
       !member_owner_info->mangle_arguments.empty()))) {
    argument_ctx_storage = *mangle_ctx;
    if(member_owner_info &&
       !member_owner_info->mangle_parameters.empty() &&
       !member_owner_info->mangle_arguments.empty()) {
      argument_ctx_storage.owner_template_parameters =
          &member_owner_info->mangle_parameters;
      argument_ctx_storage.owner_template_arguments =
          &member_owner_info->mangle_arguments;
      if(!member_owner_info->pack_sizes.empty()) {
        argument_ctx_storage.template_argument_pack_sizes =
            &member_owner_info->pack_sizes;
      }
    }
    argument_ctx_storage.prefer_source_template_name_prefixes_in_expressions =
        false;
    argument_ctx = &argument_ctx_storage;
  }
  if(!build_dependent_template_arguments_ir(dependent_arguments,
                                            parameters,
                                            argument_ctx,
                                            arguments)) {
      return false;
  }

  if(class_template) {
    const semantic_model::ClassInfo * member_owner =
        member_owner_template_scope;
    shared_ptr<const ClassTemplateSpecializationMangleInfo> owner_info =
        member_owner_info;
    if(member_owner && member_owner->type && owner_info) {
      TypeMangleContext owner_ctx_storage;
      const TypeMangleContext * owner_ctx = mangle_ctx;
      if(!owner_info->pack_sizes.empty() || !owner_info->arguments.empty()) {
        owner_ctx_storage = mangle_ctx ? *mangle_ctx : TypeMangleContext();
        if(!owner_info->pack_sizes.empty()) {
          owner_ctx_storage.template_argument_pack_sizes =
              &owner_info->pack_sizes;
        }
        owner_ctx_storage.prefer_concrete_non_type_values_for_dependent_parameter_types = true;
        owner_ctx = &owner_ctx_storage;
      }

      abi_mangle::Type owner;
      bool have_owner = try_build_type_ir(member_owner->type, owner_ctx, owner);
      if(have_owner) {
        const string owner_text = selected_named_type_text(member_owner->type);
	        const string member_template_name_substitution =
	            owner_text.empty() ?
	                string() :
	                append_qualified_component_text(owner_text, base_name);
	        abi_mangle::Type ir_type =
	            abi_mangle::Type::member_class_template_specialization(
	                std::move(owner),
	                base_name,
	                member_template_name_substitution,
	                std::move(arguments));
        abi_mangle::Type::ensure_name_metadata(ir_type)
            .register_member_expression_template_name = true;
        string prefix;
        scope_prefix_text_for_template_decl(class_template->declaring_scope,
                                            prefix);
        if(!owner_info->template_scope_prefix.empty() &&
           owner_info->template_scope_prefix.find('<') == string::npos &&
           owner_info->template_scope_prefix.find('>') == string::npos &&
           (trim_space(owner_info->template_name).empty() ||
            trim_space(owner_info->template_name) == class_template->name)) {
          prefix = owner_info->template_scope_prefix;
        }
        abi_mangle::set_substitution(
            ir_type,
            abi_mangle::SubstitutionKey::named(
                dependent_class_template_name_key_text(prefix,
                                                       base_name,
                                                       dependent_arguments)));
        out = std::move(ir_type);
        return true;
      }
    }
  }

  string standard_substitution;
  bool standard_substitution_includes_arguments = false;
  vector<TemplateArgument> source_arguments =
      template_arguments_for_dependent_mangling(dependent_arguments,
                                                parameters,
                                                mangle_ctx);
	  structured_std_standard_substitution_for_template_component(
	      base_name,
	      source_arguments,
      canonical_prefix,
      mangle_ctx,
	      standard_substitution,
	      standard_substitution_includes_arguments);

  vector<abi_mangle::SubstitutionKey> argument_keys;
  const bool needs_argument_substitution =
      !standard_substitution_includes_arguments &&
      !(mangle_ctx && mangle_ctx->suppress_type_substitution_keys);
  const bool have_argument_keys =
      needs_argument_substitution &&
      build_class_template_argument_ir_substitution_keys(arguments,
                                                        argument_keys);
  const string template_name_substitution =
      standard_substitution.empty() ?
          append_qualified_component_text(canonical_prefix,
                                          canonical_component_text(base_name)) :
          string();
  abi_mangle::Type ir_type =
      abi_mangle::Type::class_template_specialization(
          std::move(prefix_components),
          base_name,
          std::move(template_name_substitution),
          std::move(arguments),
          std::move(standard_substitution),
          standard_substitution_includes_arguments);
  if(have_argument_keys) {
    abi_mangle::set_substitution(
        ir_type,
        abi_mangle::SubstitutionKey::class_template_specialization(
            0,
            append_qualified_component_text(canonical_prefix,
                                            canonical_component_text(base_name)),
            std::move(argument_keys)));
  } else if(needs_argument_substitution) {
    abi_mangle::set_substitution(
        ir_type,
        abi_mangle::SubstitutionKey::named(
            dependent_class_template_name_key_text(canonical_prefix,
                                                   base_name,
                                                   dependent_arguments)));
  }
  out = std::move(ir_type);
  return true;
}

static bool build_template_id_class_arguments_ir_for_member_type(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx,
    vector<abi_mangle::Type::ClassTemplateArgument> & out,
    const vector<TemplateParameterInfo> * parameter_override = nullptr)
{
  vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
  dependent_arguments.reserve(syntax.argument_syntaxes.size());
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    DependentAliasTemplateArgumentSyntax argument;
    argument.syntax = syntax.argument_syntaxes[i];
    argument.text = argument.syntax.text;
    if(argument.text.empty() && i < syntax.arguments.size()) {
      argument.text = syntax.arguments[i];
      argument.syntax.text = argument.text;
    }
    if(argument.syntax.type_id && argument.syntax.type_id->semantic_type) {
      argument.type = argument.syntax.type_id->semantic_type;
    } else if(argument.syntax.resolved_type) {
      argument.type = argument.syntax.resolved_type;
    }
    dependent_arguments.push_back(std::move(argument));
  }

  const vector<TemplateParameterInfo> * raw_parameters =
      parameter_override ? parameter_override :
                           lookup_template_parameters_for_template_id_syntax(
                               syntax,
                               mangle_ctx);
  if(raw_parameters && dependent_arguments.size() < raw_parameters->size()) {
    dependent_arguments =
        complete_dependent_alias_template_arguments_for_mangling(
            *raw_parameters,
            dependent_arguments,
            template_id_default_argument_scope_for_mangling(syntax,
                                                            mangle_ctx));
  }
  vector<TemplateParameterInfo> explicit_parameter_storage;
  const vector<TemplateParameterInfo> * parameters =
      template_id_parameters_for_ir(syntax,
                                    raw_parameters,
                                    false,
                                    explicit_parameter_storage);
  return build_dependent_template_arguments_ir(dependent_arguments,
                                               parameters,
                                               mangle_ctx,
                                               out);
}

static bool template_id_resolves_to_namespace_template_for_mangling(
    const TemplateIdSyntax & syntax,
    const TypeMangleContext * mangle_ctx)
{
  if(const semantic_model::ClassTemplateDecl * class_template =
         lookup_class_template_for_template_id_syntax(syntax, mangle_ctx)) {
    return nearest_member_class_scope(class_template->declaring_scope) == nullptr;
  }
  if(const semantic_model::AliasTemplateDecl * alias_template =
         lookup_alias_template_for_template_id_syntax(syntax, mangle_ctx)) {
    return nearest_member_class_scope(alias_template->declaring_scope) == nullptr;
  }
  return false;
}

static const semantic_model::ClassTemplateDecl *
find_member_class_template_decl_for_mangling(
    const semantic_model::ClassTemplateDecl * owner_template,
    const string & name)
{
  if(!owner_template || name.empty()) {
    return nullptr;
  }
  if(owner_template->pattern_scope) {
    if(const semantic_model::ClassTemplateDecl * found =
           find_unqualified_class_template_in_mangle_scope(
               *owner_template->pattern_scope,
               name)) {
      return found;
    }
  }
  if(owner_template->declaring_scope) {
    if(const semantic_model::ClassTemplateDecl * found =
           find_unqualified_class_template_in_mangle_scope(
               *owner_template->declaring_scope,
               name)) {
      if(nearest_member_class_scope(found->declaring_scope)) {
        return found;
      }
    }
  }
  return nullptr;
}

static const semantic_model::AliasTemplateDecl *
find_member_alias_template_decl_for_mangling(
    const semantic_model::ClassTemplateDecl * owner_template,
    const string & name)
{
  if(!owner_template || name.empty()) {
    return nullptr;
  }
  if(owner_template->pattern_scope) {
    if(const semantic_model::AliasTemplateDecl * found =
           find_unqualified_alias_template_in_mangle_scope(
               *owner_template->pattern_scope,
               name)) {
      return found;
    }
  }
  if(owner_template->declaring_scope) {
    if(const semantic_model::AliasTemplateDecl * found =
           find_unqualified_alias_template_in_mangle_scope(
               *owner_template->declaring_scope,
               name)) {
      if(nearest_member_class_scope(found->declaring_scope)) {
        return found;
      }
    }
  }
  return nullptr;
}

static bool try_build_qualified_owner_type_ir(
    const QualifiedName & qualified,
    const std::vector<TemplateIdSyntax> * qualifier_template_id_syntaxes,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out,
    bool * close_owner = nullptr,
    bool expand_alias_templates = true,
    bool suppress_current_pack_grouping = false,
    bool register_template_parameter_substitution = true);

static bool try_build_qualified_template_id_type_ast_ir(
    const CppAstNode & node,
    const TemplateIdSyntax & template_id,
    const QualifiedName & qualified,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(qualified.qualifiers.empty() || qualified.name.empty()) {
    return false;
  }
  if(template_id_resolves_to_namespace_template_for_mangling(template_id,
                                                             mangle_ctx)) {
    return false;
  }

  abi_mangle::Type current;
  if(!try_build_qualified_owner_type_ir(qualified,
                                        &node.qualifier_template_id_syntaxes.as_vector(),
                                        mangle_ctx,
                                        current)) {
    return false;
  }

  const string base_name = trim_space(template_id.name.name);
  if(base_name.empty()) {
    return false;
  }
  const semantic_model::ClassTemplateDecl * owner_template_decl = nullptr;
  for(size_t i = node.qualifier_template_id_syntaxes.size(); i > 0; --i) {
    const TemplateIdSyntax & qualifier = node.qualifier_template_id_syntaxes[i - 1];
    if(qualifier.name.name.empty()) {
      continue;
    }
    owner_template_decl =
        lookup_class_template_for_template_id_syntax(qualifier, mangle_ctx);
    if(owner_template_decl) {
      break;
    }
  }
  const semantic_model::ClassTemplateDecl * member_template_decl =
      find_member_class_template_decl_for_mangling(owner_template_decl,
                                                  base_name);
  const semantic_model::AliasTemplateDecl * member_alias_template_decl =
      find_member_alias_template_decl_for_mangling(owner_template_decl,
                                                  base_name);
  const vector<TemplateParameterInfo> * member_template_parameters =
      member_alias_template_decl ? &member_alias_template_decl->parameters :
      member_template_decl && !member_template_decl->parameters.empty() ?
          &member_template_decl->parameters :
          nullptr;
  vector<abi_mangle::Type::ClassTemplateArgument> arguments;
  if(!build_template_id_class_arguments_ir_for_member_type(template_id,
                                                           mangle_ctx,
                                                           arguments,
                                                           member_template_parameters)) {
    return false;
  }

	  out = abi_mangle::Type::member_class_template_specialization(
	      std::move(current),
	      base_name,
	      string(),
	      std::move(arguments));
  attach_member_type_ir_substitution(out, node.value);
  return true;
}

static string mangle_ir_type_text_base(const string & raw)
{
  string text = trim_space(raw);
  for(;;) {
    string stripped = trim_elaborated_type_prefix(text);
    static const char dependent_alias_prefix[] = "dependent alias ";
    if(stripped.compare(0,
                        sizeof(dependent_alias_prefix) - 1,
                        dependent_alias_prefix) == 0) {
      stripped = trim_space(stripped.substr(sizeof(dependent_alias_prefix) - 1));
    }
    static const char typename_prefix[] = "typename ";
    if(stripped.compare(0, sizeof(typename_prefix) - 1, typename_prefix) == 0) {
      stripped = trim_space(stripped.substr(sizeof(typename_prefix) - 1));
    }
    if(stripped == text) {
      return text;
    }
    text = stripped;
  }
}

static bool simple_mangle_type_component(const string & raw, string & out)
{
  out = mangle_ir_type_text_base(raw);
  return !out.empty() &&
         out.find('<') == string::npos &&
         out.find('>') == string::npos &&
         is_identifier_text_for_mangling(out);
}

static bool prepend_qualified_prefix(QualifiedName & name,
                                     const QualifiedName & prefix)
{
  if(prefix.rooted || prefix.name.empty()) {
    return false;
  }
  vector<string> qualifiers = prefix.qualifiers;
  qualifiers.push_back(prefix.name);
  qualifiers.insert(qualifiers.end(),
                    name.qualifiers.begin(),
                    name.qualifiers.end());
  name.rooted = false;
  name.qualifiers.swap(qualifiers);
  return true;
}

static bool try_build_qualified_named_type_syntax_ir(
    const QualifiedName & qualified,
    const TypeMangleContext * mangle_ctx,
    bool apply_lexical_prefix,
    abi_mangle::Type & out)
{
  if(qualified.rooted || qualified.name.empty()) {
    return false;
  }

  QualifiedName effective = qualified;
  if(apply_lexical_prefix && !effective.qualifiers.empty()) {
    const string first_qualifier =
        semantic_utils::strip_trailing_top_level_template_arguments(
            trim_space(effective.qualifiers[0]));
    if(syntax_name_matches_template_parameter(first_qualifier, mangle_ctx)) {
      return false;
    }
    QualifiedName lookup_prefix;
    if(qualified_type_text_lexical_lookup_prefix(effective,
                                                 mangle_ctx,
                                                 lookup_prefix) &&
       !prepend_qualified_prefix(effective, lookup_prefix)) {
      return false;
    }
  }

  vector<string> qualifiers;
  qualifiers.reserve(effective.qualifiers.size());
  for(size_t i = 0; i < effective.qualifiers.size(); ++i) {
    string qualifier;
    if(!simple_mangle_type_component(effective.qualifiers[i], qualifier)) {
      return false;
    }
    qualifiers.push_back(qualifier);
  }

  string name;
  if(!simple_mangle_type_component(effective.name, name)) {
    return false;
  }

  vector<abi_mangle::Type::NameComponent> prefix_components;
  string canonical_prefix;
  if(!qualifiers.empty()) {
    QualifiedName prefix;
    prefix.name = qualifiers.back();
    prefix.qualifiers.assign(qualifiers.begin(), qualifiers.end() - 1);
    if(!build_name_prefix_components_ir(&prefix,
                                        prefix_components,
                                        canonical_prefix)) {
      return false;
    }
  }

  const string canonical_name =
      append_qualified_component_text(canonical_prefix,
                                      canonical_component_text(name));
	  out = abi_mangle::Type::named_type(std::move(prefix_components),
	                                     std::move(name),
	                                     canonical_name);
  abi_mangle::set_substitution(
      out,
      abi_mangle::SubstitutionKey::named(canonical_name));
  return true;
}

static bool try_build_unqualified_component_type_ir(
    const string & raw,
    const TypeMangleContext * mangle_ctx,
    bool register_template_parameter_substitution,
    abi_mangle::Type & out)
{
  const string component = mangle_ir_type_text_base(raw);
  if(component.empty()) {
    return false;
  }
  if(try_build_template_parameter_type_text_ir(component,
                                               mangle_ctx,
                                               register_template_parameter_substitution,
                                               out)) {
    return true;
  }
  if(!is_identifier_text_for_mangling(component)) {
    return false;
  }
  const string canonical_name = canonical_component_text(component);
	  out = abi_mangle::Type::named_type(
	      vector<abi_mangle::Type::NameComponent>(),
	      std::move(component),
	      canonical_name);
  abi_mangle::set_substitution(
      out,
      abi_mangle::SubstitutionKey::named(canonical_name));
  return true;
}

static bool try_build_qualified_component_type_ir(
    const string & raw,
    bool has_owner,
    const abi_mangle::Type & owner,
    const TypeMangleContext * mangle_ctx,
    bool register_template_parameter_substitution,
    abi_mangle::Type & out)
{
  if(!has_owner) {
    return try_build_unqualified_component_type_ir(
        raw,
        mangle_ctx,
        register_template_parameter_substitution,
        out);
  }
  const string component = mangle_ir_type_text_base(raw);
  if(component.empty() || !is_identifier_text_for_mangling(component)) {
    return false;
  }
  out = abi_mangle::Type::member_named_type(owner, component, string());
  return true;
}

static bool type_ir_contains_class_template_specialization(
    const abi_mangle::Type & type)
{
  if(type.kind == abi_mangle::Type::TK_CLASS_TEMPLATE_SPECIALIZATION) {
    return true;
  }
  if(type.inner && type_ir_contains_class_template_specialization(*type.inner)) {
    return true;
  }
  if(type.owner && type_ir_contains_class_template_specialization(*type.owner)) {
    return true;
  }
  if(type.name_owner &&
     type_ir_contains_class_template_specialization(*type.name_owner)) {
    return true;
  }
  for(size_t i = 0; i < type.params.size(); ++i) {
    if(type_ir_contains_class_template_specialization(type.params[i])) {
      return true;
    }
  }
  return false;
}

static bool try_build_qualified_owner_type_ir(
    const QualifiedName & qualified,
    const std::vector<TemplateIdSyntax> * qualifier_template_id_syntaxes,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out,
    bool * close_owner,
    bool expand_alias_templates,
    bool suppress_current_pack_grouping,
    bool register_template_parameter_substitution)
{
  if(qualified.qualifiers.empty()) {
    return false;
  }

  const auto later_qualifier_template_id =
      [qualifier_template_id_syntaxes, &qualified](size_t index) -> bool
      {
        if(!qualifier_template_id_syntaxes) {
          return false;
        }
        for(size_t j = index + 1;
            j < qualified.qualifiers.size() &&
            j < qualifier_template_id_syntaxes->size();
            ++j) {
          if(!(*qualifier_template_id_syntaxes)[j].name.name.empty()) {
            return true;
          }
        }
        return false;
      };

  abi_mangle::Type current;
  bool has_current = false;
  const semantic_model::ClassTemplateDecl * current_class_template_decl = nullptr;
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    abi_mangle::Type component;
    const semantic_model::ClassTemplateDecl * component_class_template_decl =
        nullptr;
    const TemplateIdSyntax * qualifier_template_id =
        qualifier_template_id_syntaxes &&
                i < qualifier_template_id_syntaxes->size() &&
                !(*qualifier_template_id_syntaxes)[i].name.name.empty() ?
            &(*qualifier_template_id_syntaxes)[i] :
            nullptr;
    if(qualifier_template_id) {
      const string qualifier_base_name =
              strip_leading_template_disambiguator(
              qualifier_template_id->name.name);
      if(has_current) {
        component_class_template_decl =
            find_member_class_template_decl_for_mangling(
                current_class_template_decl,
                qualifier_base_name);
        const vector<TemplateParameterInfo> * member_template_parameters =
            component_class_template_decl ?
                &component_class_template_decl->parameters :
                nullptr;
        vector<abi_mangle::Type::ClassTemplateArgument> arguments;
        TemplateIdSyntax member_template_id = *qualifier_template_id;
        member_template_id.name.rooted = false;
        member_template_id.name.qualifiers.clear();
        member_template_id.name.name = qualifier_base_name;
        if(!build_template_id_class_arguments_ir_for_member_type(
               member_template_id,
               mangle_ctx,
               arguments,
               member_template_parameters)) {
          return false;
        }
        const string base_name = qualifier_base_name;
        if(base_name.empty()) {
          return false;
        }
	        component = abi_mangle::Type::member_class_template_specialization(
	            current,
	            base_name,
	            string(),
	            std::move(arguments));
        abi_mangle::Type::ensure_name_metadata(component)
            .register_member_expression_template_name = true;
        attach_member_type_ir_substitution(component, string());
      } else {
        component_class_template_decl =
            lookup_class_template_for_template_id_syntax(*qualifier_template_id,
                                                         mangle_ctx);
        if(!try_build_template_id_type_ir(*qualifier_template_id,
                                          mangle_ctx,
                                          component,
                                          expand_alias_templates,
                                          suppress_current_pack_grouping)) {
          return false;
        }
      }
    } else if(!has_current && later_qualifier_template_id(i)) {
      continue;
    } else if(!try_build_qualified_component_type_ir(qualified.qualifiers[i],
                                                     has_current,
                                                     current,
                                                     mangle_ctx,
                                                     register_template_parameter_substitution,
                                                     component)) {
      return false;
    }
    if(has_current && qualifier_template_id && !component.name_owner) {
      component.name_owner.reset(new abi_mangle::Type(current));
    }
	    current = std::move(component);
    current_class_template_decl = component_class_template_decl;
    has_current = true;
  }
  if(!has_current) {
    return false;
  }
  out = std::move(current);
  if(close_owner) {
    *close_owner = type_ir_contains_class_template_specialization(out);
  }
  return true;
}

static bool pack_expansion_text_resolves_to_concrete_owner_pack(
    const string & text,
    const TypeMangleContext * mangle_ctx);

static bool try_build_direct_type_syntax_text_ir(
    const string & raw_text,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out,
    bool register_template_parameter_substitution)
{
  string text = mangle_ir_type_text_base(raw_text);
  if(text.empty()) {
    return false;
  }

  if(text.size() >= 3 &&
     text.compare(text.size() - 3, 3, "...") == 0) {
    abi_mangle::Type inner;
    if(!try_build_direct_type_syntax_text_ir(
           trim_space(text.substr(0, text.size() - 3)),
           mangle_ctx,
           inner,
           register_template_parameter_substitution)) {
      return false;
    }
    if(pack_expansion_text_resolves_to_concrete_owner_pack(text, mangle_ctx)) {
      out = inner;
      return true;
    }
    out = abi_mangle::Type::pack_expansion(std::move(inner));
    return attach_context_free_type_ir_substitution(out);
  }

  bool cv_const = false;
  bool cv_volatile = false;
  string cv_base = text;
  bool stripped_cv = true;
  while(stripped_cv) {
    stripped_cv = false;
    static const char const_prefix[] = "const ";
    static const char volatile_prefix[] = "volatile ";
    if(cv_base.compare(0, sizeof(const_prefix) - 1, const_prefix) == 0) {
      cv_const = true;
      cv_base = trim_space(cv_base.substr(sizeof(const_prefix) - 1));
      stripped_cv = true;
      continue;
    }
    if(cv_base.compare(0, sizeof(volatile_prefix) - 1, volatile_prefix) == 0) {
      cv_volatile = true;
      cv_base = trim_space(cv_base.substr(sizeof(volatile_prefix) - 1));
      stripped_cv = true;
      continue;
    }
    static const char const_suffix[] = " const";
    static const char volatile_suffix[] = " volatile";
    if(cv_base.size() > sizeof(const_suffix) - 1 &&
       cv_base.compare(cv_base.size() - (sizeof(const_suffix) - 1),
                       sizeof(const_suffix) - 1,
                       const_suffix) == 0) {
      cv_const = true;
      cv_base = trim_space(cv_base.substr(0,
                                          cv_base.size() -
                                              (sizeof(const_suffix) - 1)));
      stripped_cv = true;
      continue;
    }
    if(cv_base.size() > sizeof(volatile_suffix) - 1 &&
       cv_base.compare(cv_base.size() - (sizeof(volatile_suffix) - 1),
                       sizeof(volatile_suffix) - 1,
                       volatile_suffix) == 0) {
      cv_volatile = true;
      cv_base = trim_space(cv_base.substr(0,
                                          cv_base.size() -
                                              (sizeof(volatile_suffix) - 1)));
      stripped_cv = true;
      continue;
    }
  }
  if((cv_const || cv_volatile) && cv_base != text) {
    abi_mangle::Type inner;
    if(try_build_direct_type_syntax_text_ir(cv_base,
                                            mangle_ctx,
                                            inner,
                                            register_template_parameter_substitution)) {
      out = abi_mangle::Type::cv(cv_const, cv_volatile, std::move(inner));
      return attach_context_free_type_ir_substitution(out);
    }
  }

  string builtin_code;
  if(try_mangle_builtin_text(text, builtin_code) && !builtin_code.empty()) {
    out = abi_mangle::Type::builtin(builtin_code);
    return true;
  }

  if(try_build_template_parameter_type_text_ir(
         text,
         mangle_ctx,
         register_template_parameter_substitution,
         out)) {
    return true;
  }

  QualifiedName qualified;
  if(semantic_utils::split_qualified_name_text(text, qualified) &&
     !qualified.qualifiers.empty()) {
    const bool mentions_template_parameter =
        text_mentions_template_mangle_parameter(text, mangle_ctx);
    TypePtr lookup_type = mentions_template_parameter ?
        TypePtr() :
        lookup_qualified_named_type_for_mangling(qualified, mangle_ctx);
    if(lookup_type && try_build_type_ir(lookup_type, mangle_ctx, out)) {
      return true;
    }
    QualifiedName effective = qualified;
    const string first_qualifier =
        semantic_utils::strip_trailing_top_level_template_arguments(
            trim_space(effective.qualifiers[0]));
    QualifiedName lookup_prefix;
    if(!syntax_name_matches_template_parameter(first_qualifier, mangle_ctx) &&
       qualified_type_text_lexical_lookup_prefix(effective,
                                                 mangle_ctx,
                                                 lookup_prefix) &&
       !prepend_qualified_prefix(effective, lookup_prefix)) {
      return false;
    }
    if(mentions_template_parameter) {
      abi_mangle::Type owner;
      if(try_build_qualified_owner_type_ir(effective,
                                           nullptr,
                                           mangle_ctx,
                                           owner)) {
	        out = abi_mangle::Type::member_named_type(owner,
	                                                   effective.name,
	                                                   string());
        attach_member_type_ir_substitution(out, text);
        return true;
      }
    }
    if(try_build_qualified_named_type_syntax_ir(qualified,
                                               mangle_ctx,
                                               true,
                                               out)) {
      return true;
    }
  }

  if(is_identifier_text_for_mangling(text) &&
     text.find("__local_") == string::npos &&
     text.find("(anonymous namespace)") == string::npos) {
    TypePtr lookup_type = lookup_scope_named_type_for_mangling(text, mangle_ctx);
    if(lookup_type && try_build_type_ir(lookup_type, mangle_ctx, out)) {
      return true;
    }
  }

  return try_build_unqualified_component_type_ir(
      text,
      mangle_ctx,
      register_template_parameter_substitution,
      out);
}

static bool try_build_resolved_type_argument_text_ir(
    const string & raw_text,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  const string text = mangle_ir_type_text_base(raw_text);
  if(text.empty()) {
    return false;
  }

  string builtin_code;
  if(try_mangle_builtin_text(text, builtin_code) && !builtin_code.empty()) {
    out = abi_mangle::Type::builtin(builtin_code);
    return true;
  }

  if(try_build_template_parameter_type_text_ir(text,
                                               mangle_ctx,
                                               true,
                                               out)) {
    return true;
  }

  QualifiedName qualified;
  if(semantic_utils::split_qualified_name_text(text, qualified) &&
     !qualified.qualifiers.empty()) {
    TypePtr lookup_type =
        lookup_qualified_named_type_for_mangling(qualified, mangle_ctx);
    return lookup_type && try_build_type_ir(lookup_type, mangle_ctx, out);
  }

  return false;
}

static bool pack_expansion_text_resolves_to_concrete_owner_pack(
    const string & text,
    const TypeMangleContext * mangle_ctx)
{
  if(!mangle_ctx ||
     !mangle_ctx->owner_template_parameters ||
     !mangle_ctx->owner_template_arguments) {
    return false;
  }
  string stripped = trim_elaborated_type_prefix(text);
  if(stripped.size() < 3 ||
     stripped.compare(stripped.size() - 3, 3, "...") != 0) {
    return false;
  }
  stripped = trim_space(stripped.substr(0, stripped.size() - 3));
  const vector<TemplateParameterInfo> & parameters =
      *mangle_ctx->owner_template_parameters;
  const vector<TemplateArgument> & arguments =
      *mangle_ctx->owner_template_arguments;
  for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
    const TemplateParameterInfo & parameter = parameters[i];
    const TemplateArgument & argument = arguments[i];
    if(parameter.kind != TemplateParameterInfo::TP_TYPE ||
       !parameter.parameter_pack ||
       parameter.name.empty() ||
       argument.kind != TemplateArgument::TA_TYPE ||
       !argument.type ||
       !contains_identifier_token(stripped, parameter.name)) {
      continue;
    }
    if(owner_template_argument_index_is_suppressed(mangle_ctx, i) ||
       template_argument_is_self_type_parameter(argument,
                                                parameter,
                                                TypePtr(),
                                                parameter.name)) {
      continue;
    }
    return true;
  }
  return false;
}

static bool try_build_concrete_owner_pack_expansion_type_ir(
    const string & text,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(!mangle_ctx ||
     !mangle_ctx->owner_template_parameters ||
     !mangle_ctx->owner_template_arguments) {
    return false;
  }
  string stripped = trim_elaborated_type_prefix(text);
  if(stripped.size() < 3 ||
     stripped.compare(stripped.size() - 3, 3, "...") != 0) {
    return false;
  }
  stripped = trim_space(stripped.substr(0, stripped.size() - 3));
  if(stripped.size() > 2 &&
     stripped.compare(stripped.size() - 2, 2, "&&") == 0) {
    stripped = trim_space(stripped.substr(0, stripped.size() - 2));
  } else if(!stripped.empty() && stripped[stripped.size() - 1] == '&') {
    stripped = trim_space(stripped.substr(0, stripped.size() - 1));
  }
  static const char const_prefix[] = "const ";
  if(stripped.compare(0, sizeof(const_prefix) - 1, const_prefix) == 0) {
    stripped = trim_space(stripped.substr(sizeof(const_prefix) - 1));
  }
  static const char volatile_prefix[] = "volatile ";
  if(stripped.compare(0, sizeof(volatile_prefix) - 1, volatile_prefix) == 0) {
    stripped = trim_space(stripped.substr(sizeof(volatile_prefix) - 1));
  }

  const vector<TemplateParameterInfo> & parameters =
      *mangle_ctx->owner_template_parameters;
  const vector<TemplateArgument> & arguments =
      *mangle_ctx->owner_template_arguments;
  for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
    const TemplateParameterInfo & parameter = parameters[i];
    const TemplateArgument & argument = arguments[i];
    if(parameter.kind != TemplateParameterInfo::TP_TYPE ||
       !parameter.parameter_pack ||
       parameter.name.empty() ||
       stripped != parameter.name ||
       argument.kind != TemplateArgument::TA_TYPE ||
       !argument.type) {
      continue;
    }
    if(owner_template_argument_index_is_suppressed(mangle_ctx, i) ||
       template_argument_is_self_type_parameter(argument,
                                                parameter,
                                                TypePtr(),
                                                parameter.name)) {
      continue;
    }
    TypeMangleContext owner_arg_ctx_storage;
    return try_build_template_argument_type_ir(
        argument.type,
        suppress_owner_template_argument_index(mangle_ctx,
                                               i,
                                               owner_arg_ctx_storage),
        out);
  }
  return false;
}

static bool qualified_node_has_template_id_qualifier_syntax(
    const CppAstNode & node)
{
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(!node.qualifier_template_id_syntaxes[i].name.name.empty()) {
      return true;
    }
  }
  return false;
}

static bool try_build_lexically_prefixed_qualified_type_ast_ir(
    const CppAstNode & node,
    const QualifiedName & qualified,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(qualified.rooted ||
     qualified.qualifiers.empty() ||
     qualified.name.empty() ||
     qualified_node_has_template_id_qualifier_syntax(node) ||
     ast_node_mentions_direct_template_parameter(node, mangle_ctx)) {
    return false;
  }
  const string first_qualifier =
      semantic_utils::strip_trailing_top_level_template_arguments(
          trim_space(qualified.qualifiers[0]));
  if(syntax_name_matches_template_parameter(first_qualifier, mangle_ctx)) {
    return false;
  }

  QualifiedName lookup_prefix;
  if(!qualified_type_text_lexical_lookup_prefix(qualified,
                                                mangle_ctx,
                                                lookup_prefix)) {
    return false;
  }

  QualifiedName prefixed = qualified;
  if(!prepend_qualified_prefix(prefixed, lookup_prefix)) {
    return false;
  }
  return try_build_qualified_named_type_syntax_ir(prefixed,
                                                  mangle_ctx,
                                                  false,
                                                  out);
}

static bool try_build_dependent_class_template_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  void * class_template_decl = nullptr;
  vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
  if(!named_type_dependent_class_template(type,
                                          class_template_decl,
                                          dependent_arguments) ||
     !dependent_class_template_metadata_should_drive_mangling(type, mangle_ctx)) {
    return false;
  }
  const semantic_model::ClassTemplateDecl * class_template =
      static_cast<const semantic_model::ClassTemplateDecl *>(class_template_decl);
  if(!class_template) {
    return false;
  }

  QualifiedName prefix;
  vector<abi_mangle::Type::NameComponent> prefix_components;
  string canonical_prefix;
  const QualifiedName * prefix_ptr =
      scope_prefix_syntax_for_template_decl(class_template->declaring_scope,
                                            prefix) ?
          &prefix :
          nullptr;
  if(!build_name_prefix_components_ir(prefix_ptr,
                                      prefix_components,
                                      canonical_prefix)) {
    return false;
  }

  vector<abi_mangle::Type::ClassTemplateArgument> arguments;
  if(!build_dependent_template_arguments_ir(dependent_arguments,
                                            &class_template->parameters,
                                            mangle_ctx,
                                            arguments)) {
    return false;
  }

  const string base_name = trim_space(class_template->name);
  vector<abi_mangle::SubstitutionKey> argument_keys;
  const bool needs_argument_substitution =
      !(mangle_ctx && mangle_ctx->suppress_type_substitution_keys);
  const bool have_argument_keys =
      needs_argument_substitution &&
      build_class_template_argument_ir_substitution_keys(arguments,
                                                        argument_keys);
  const string template_name_substitution =
      append_qualified_component_text(canonical_prefix,
                                      canonical_component_text(base_name));
  abi_mangle::Type ir_type =
      abi_mangle::Type::class_template_specialization(
          std::move(prefix_components),
          base_name,
          std::move(template_name_substitution),
          std::move(arguments),
          string(),
          false);
  if(have_argument_keys) {
    abi_mangle::set_substitution(
        ir_type,
        abi_mangle::SubstitutionKey::class_template_specialization(
            0,
            append_qualified_component_text(canonical_prefix,
                                            canonical_component_text(base_name)),
            std::move(argument_keys)));
  } else if(needs_argument_substitution) {
    abi_mangle::set_substitution(
        ir_type,
        abi_mangle::SubstitutionKey::named(
            dependent_class_template_name_key_text(canonical_prefix,
                                                   base_name,
                                                   dependent_arguments)));
  }
  out = std::move(ir_type);
  return true;
}

static bool try_build_dependent_template_template_parameter_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  string parameter_name;
  size_t parameter_arity = static_cast<size_t>(-1);
  vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
  if(!named_type_dependent_template_template_parameter(type,
                                                       parameter_name,
                                                       parameter_arity,
                                                       dependent_arguments)) {
    return false;
  }

  size_t template_parameter_index = 0;
  const TemplateParameterInfo * template_parameter = nullptr;
  const bool found_template_parameter_index =
      try_find_template_template_parameter_index(parameter_name,
                                                mangle_ctx,
                                                template_parameter_index,
                                                template_parameter);
  if(!found_template_parameter_index &&
     (!mangle_ctx ||
      (!mangle_ctx->template_parameters && !mangle_ctx->owner_template_parameters))) {
    return false;
  }

  vector<TemplateParameterInfo> template_template_parameters;
  const vector<TemplateParameterInfo> * argument_parameters = nullptr;
  if(found_template_parameter_index &&
     template_parameter &&
     template_parameter->template_parameter_count != 0 &&
     template_parameter->template_parameter_count != static_cast<size_t>(-1)) {
    template_template_parameters.resize(
        template_parameter->template_parameter_count);
    argument_parameters = &template_template_parameters;
  } else if(parameter_arity != static_cast<size_t>(-1)) {
    template_template_parameters.resize(parameter_arity);
    argument_parameters = &template_template_parameters;
  }

  vector<abi_mangle::Type::ClassTemplateArgument> arguments;
  if(!build_dependent_template_arguments_ir(dependent_arguments,
                                            argument_parameters,
                                            mangle_ctx,
                                            arguments)) {
    return false;
  }

  if(found_template_parameter_index) {
    out = abi_mangle::Type::template_parameter_class_template_specialization(
        template_parameter_index,
        std::move(arguments));
  } else {
    const string template_name = trim_space(parameter_name);
    if(template_name.empty() ||
       template_name.find("::") != string::npos ||
       !is_identifier_text_for_mangling(template_name)) {
      return false;
    }
    const string canonical_name = canonical_component_text(template_name);
    out = abi_mangle::Type::class_template_specialization(
        vector<abi_mangle::Type::NameComponent>(),
        template_name,
        canonical_name,
        std::move(arguments),
        string(),
        false);
    attach_type_ir_substitution(out);
  }
  return true;
}

static bool try_build_dependent_qualified_member_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  TypePtr owner;
  vector<string> members;
  bool leading_typename = false;
  vector<TemplateIdSyntax> member_template_ids;
  if(!named_type_dependent_qualified_member(type,
                                            owner,
                                            members,
                                            leading_typename,
                                            &member_template_ids) ||
     members.empty()) {
    return false;
  }
  (void)leading_typename;

  abi_mangle::Type owner_ir;
  if(!try_build_type_ir(owner, mangle_ctx, owner_ir)) {
    return false;
  }
  abi_mangle::Type current = owner_ir;
  for(size_t i = 0; i < members.size(); ++i) {
    const string member = trim_space(members[i]);
    if(member.empty()) {
      return false;
    }
    if(i < member_template_ids.size() &&
       !member_template_ids[i].name.name.empty()) {
      abi_mangle::Type member_template;
      TemplateIdSyntax member_template_id = member_template_ids[i];
      member_template_id.name.rooted = false;
      member_template_id.name.qualifiers.clear();
      member_template_id.name.name =
          strip_leading_template_disambiguator(member_template_id.name.name);
      if(!try_build_template_id_type_ir(member_template_id,
                                        mangle_ctx,
                                        member_template)) {
        return false;
      }
      member_template.name_owner.reset(new abi_mangle::Type(current));
      attach_member_type_ir_substitution(member_template, string());
      current = member_template;
    } else {
      current = abi_mangle::Type::member_named_type(std::move(current),
	                                                member,
	                                                string());
      attach_member_type_ir_substitution(current, string());
    }
  }
  out = std::move(current);
  return true;
}

static bool try_build_dependent_alias_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  TypePtr base = strip_top_level_cv(type);
  if(base &&
     base->kind == Type::TK_NAMED &&
     base->named_semantic_kind == Type::NSK_DEPENDENT_ALIAS &&
     try_build_typed_member_named_type_ir(base, mangle_ctx, out)) {
    return true;
  }

  void * alias_template_decl = nullptr;
  vector<DependentAliasTemplateArgumentSyntax> arguments;
  if(named_type_dependent_alias_template(type, alias_template_decl, arguments)) {
    const semantic_model::AliasTemplateDecl * alias_template =
        static_cast<const semantic_model::AliasTemplateDecl *>(alias_template_decl);
    if(alias_template && alias_template->type_id &&
       arguments.size() <= alias_template->parameters.size()) {
      arguments = complete_dependent_alias_template_arguments_for_mangling(
          alias_template->parameters,
          arguments,
          alias_template->declaring_scope);

      CppAstNode alias_expansion =
          clone_ast_node_for_mangling(*alias_template->type_id);
      substitute_dependent_alias_template_arguments_in_node(
          alias_expansion,
          alias_template->parameters,
          arguments);

      TypeMangleContext alias_ctx_storage;
      const TypeMangleContext * alias_ctx = mangle_ctx;
      if(mangle_ctx) {
        alias_ctx_storage = *mangle_ctx;
        ++alias_ctx_storage.alias_expansion_depth;
        alias_ctx_storage.suppress_current_type_id_substitution_registration = true;
        alias_ctx = &alias_ctx_storage;
      }
      if(try_build_type_id_ast_ir(alias_expansion, alias_ctx, out)) {
        return true;
      }
    }
  }

  if(base &&
     base->kind == Type::TK_NAMED &&
     base->named_semantic_kind == Type::NSK_DEPENDENT_ALIAS &&
     base->named_dependent_type_expression_node) {
    TypeMangleContext alias_ctx_storage;
    const TypeMangleContext * alias_ctx = mangle_ctx;
    if(mangle_ctx) {
      alias_ctx_storage = *mangle_ctx;
      ++alias_ctx_storage.alias_expansion_depth;
      alias_ctx_storage.suppress_current_type_id_substitution_registration = true;
      alias_ctx = &alias_ctx_storage;
    }
    return try_build_type_id_ast_ir(*base->named_dependent_type_expression_node,
                                    alias_ctx,
                                    out);
  }

  return false;
}

static bool try_build_dependent_decltype_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base ||
     base->kind != Type::TK_NAMED ||
     base->named_semantic_kind != Type::NSK_DEPENDENT_DECLTYPE ||
     !base->named_dependent_type_expression_node) {
    return false;
  }
  TypeMangleContext decltype_ctx_storage;
  const TypeMangleContext * decltype_ctx = mangle_ctx;
  if(mangle_ctx) {
    decltype_ctx_storage = *mangle_ctx;
  }
  decltype_ctx_storage.suppress_decltype_callee_template_prefix_substitution = true;
  decltype_ctx = &decltype_ctx_storage;
  abi_mangle::DependentExpression expression;
  if(!try_build_dependent_expression_ir(
         *base->named_dependent_type_expression_node,
         decltype_ctx,
         expression)) {
    return false;
  }
  abi_mangle::Type ir_type;
  ir_type.kind = abi_mangle::Type::TK_DECLTYPE_EXPRESSION;
  ir_type.expression.reset(new abi_mangle::DependentExpression(expression));
  attach_type_ir_substitution(ir_type);
  out = std::move(ir_type);
  return true;
}

static bool try_build_cast_target_type_id_ast_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(node.kind != CppAstKind::type_id || node.children.empty()) {
    return false;
  }

  const CppAstNode * abstract = nullptr;
  for(size_t i = 1; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::abstract_declarator) {
      abstract = &node.children[i];
      break;
    }
  }

  ETokenType reference_operator = static_cast<ETokenType>(0);
  if(abstract &&
     simple_reference_declarator_operator(*abstract, reference_operator)) {
    TypePtr owner_argument_type;
    if(owner_template_type_argument_for_specifier_seq(node.children[0],
                                                      mangle_ctx,
                                                      owner_argument_type)) {
      return try_build_type_ir(remove_reference_type(owner_argument_type),
                               mangle_ctx,
                               out);
    }
    return try_build_type_specifier_seq_ast_ir(node.children[0],
                                               mangle_ctx,
                                               out);
  }

  return try_build_type_id_ast_ir(node, mangle_ctx, out);
}

static bool try_build_dependent_builtin_type_transform_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(!type ||
     type->kind != Type::TK_NAMED ||
     type->named_semantic_kind != Type::NSK_DEPENDENT_TYPE ||
     !type->inner) {
    return false;
  }

  static const char prefix[] = "$builtin-type-transform:";
  const string payload = named_type_semantic_payload(type);
  if(payload.compare(0, sizeof(prefix) - 1, prefix) != 0) {
    return false;
  }
  const size_t name_begin = sizeof(prefix) - 1;
  const size_t name_end = payload.find('|', name_begin);
  const string builtin_name =
      payload.substr(name_begin,
                     name_end == string::npos ? string::npos :
                                                name_end - name_begin);
  if(builtin_name.empty() ||
     !is_mangleable_builtin_type_transform_name(builtin_name)) {
    return false;
  }

  abi_mangle::Type argument;
  if(!try_build_type_ir(type->inner, mangle_ctx, argument)) {
    return false;
  }
  out = abi_mangle::Type::builtin_type_transform(builtin_name,
                                                 std::move(argument));
  return true;
}

static const Type::LambdaMangleMetadata * named_type_lambda_mangle_metadata(
    const TypePtr & type)
{
  if(!type ||
     type->kind != Type::TK_NAMED ||
     !type->named_lambda_mangle) {
    return nullptr;
  }
  const Type::LambdaMangleMetadata * metadata =
      type->named_lambda_mangle.get();
  const bool function_context =
      metadata->context_function_type &&
      metadata->context_function_symbol_options &&
      !metadata->context_function_qualified_name.name.empty();
  const bool namespace_context =
      !metadata->local_source_name.empty() &&
      !function_context;
  return function_context || namespace_context ? metadata : nullptr;
}

static bool lambda_metadata_has_function_context(
    const Type::LambdaMangleMetadata & metadata)
{
  return metadata.context_function_type &&
         metadata.context_function_symbol_options &&
         !metadata.context_function_qualified_name.name.empty();
}

static bool itanium_lambda_context_options_from_type(
    const TypePtr & type,
    shared_ptr<FunctionSymbolOptions> & out)
{
  const Type::LambdaMangleMetadata * metadata =
      named_type_lambda_mangle_metadata(type);
  if(!metadata || !lambda_metadata_has_function_context(*metadata)) {
    return false;
  }
  shared_ptr<FunctionSymbolOptions> context_options =
      static_pointer_cast<FunctionSymbolOptions>(
          metadata->context_function_symbol_options);
  if(!context_options) {
    return false;
  }
  out.swap(context_options);
  return true;
}

static bool lambda_context_uses_clang_local_source_name(
    const FunctionSymbolOptions & options)
{
  return !options.is_member_function &&
         !options.template_parameters &&
         (!options.template_arguments || options.template_arguments->empty());
}

static string clang_local_lambda_source_name(const string & discriminator)
{
  return string("$_") + (discriminator.empty() ? string("0") : discriminator);
}

static string local_entity_source_name(
    const Type::LambdaMangleMetadata & metadata,
    const FunctionSymbolOptions & context_options)
{
  if(!metadata.local_source_name.empty()) {
    return metadata.local_source_name;
  }
  return lambda_context_uses_clang_local_source_name(context_options) ?
      clang_local_lambda_source_name(metadata.discriminator) :
      string();
}

static bool build_itanium_lambda_context_fragment_from_type(
    const TypePtr & type,
    string & out,
    vector<abi_mangle::SubstitutionSlot> & substitution_slots,
    shared_ptr<abi_mangle::FunctionEncoding> * context_function = nullptr)
{
  if(context_function) {
    context_function->reset();
  }
  shared_ptr<FunctionSymbolOptions> context_options;
  if(!itanium_lambda_context_options_from_type(type, context_options)) {
    return false;
  }

  string function_encoding;
  vector<abi_mangle::SubstitutionSlot> context_substitution_slots;
  const Type::LambdaMangleMetadata * metadata =
      named_type_lambda_mangle_metadata(type);
  if(!metadata) {
    return false;
  }
  if(!emit_itanium_function_encoding_with_substitutions(
         metadata->context_function_qualified_name,
         metadata->context_function_display_name,
         metadata->context_function_type,
         *context_options,
         function_encoding,
         &context_substitution_slots)) {
    return false;
  }
  const bool main_context =
      metadata->context_function_qualified_name.name == "main" &&
      metadata->context_function_qualified_name.qualifiers.empty() &&
      !metadata->context_function_qualified_name.rooted &&
      !context_options->is_member_function &&
      (!context_options->template_parameters ||
       context_options->template_parameters->empty()) &&
      (!context_options->template_arguments ||
       context_options->template_arguments->empty());
  if(main_context) {
    TypePtr context_function_type =
        strip_top_level_cv(metadata->context_function_type);
    if(context_function_type &&
       context_function_type->kind == Type::TK_FUNCTION &&
       context_function_type->params.empty() &&
       !context_function_type->variadic) {
      function_encoding = "4main";
      context_substitution_slots.clear();
    }
  }

  if(context_function && !main_context) {
    abi_mangle::FunctionEncoding function_ir;
    if(build_itanium_function_context_encoding_ir(
           metadata->context_function_qualified_name,
           metadata->context_function_display_name,
           metadata->context_function_type,
           *context_options,
           function_ir)) {
      context_function->reset(
          new abi_mangle::FunctionEncoding(function_ir));
    }
  }

  string candidate = "Z" + function_encoding + "E";
  out.swap(candidate);
  substitution_slots.swap(context_substitution_slots);
  return true;
}

static bool build_itanium_lambda_signature_parameter_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    vector<abi_mangle::Type> & out)
{
  const Type::LambdaMangleMetadata * metadata =
      named_type_lambda_mangle_metadata(type);
  if(!metadata) {
    return false;
  }

  vector<abi_mangle::Type> parameter_types;
  parameter_types.reserve(metadata->signature_parameter_types.size());
  for(size_t i = 0; i < metadata->signature_parameter_types.size(); ++i) {
    abi_mangle::Type parameter_type;
    if(!try_build_type_ir(metadata->signature_parameter_types[i],
                          mangle_ctx,
                          parameter_type)) {
      return false;
    }
    parameter_types.push_back(parameter_type);
  }

  out.swap(parameter_types);
  return true;
}

static bool initialize_local_entity_function_metadata(
    const TypePtr & local_entity_type,
    const TypeMangleContext * signature_mangle_ctx,
    abi_mangle::FunctionEncoding & function)
{
  const Type::LambdaMangleMetadata * metadata =
      named_type_lambda_mangle_metadata(local_entity_type);
  if(!metadata) {
    return false;
  }
  if(!lambda_metadata_has_function_context(*metadata)) {
    if(metadata->local_source_name.empty()) {
      return false;
    }
    abi_mangle::FunctionEncoding::LambdaMetadata & lambda =
        abi_mangle::FunctionEncoding::ensure_lambda_metadata(function);
    lambda.context_fragment.clear();
    lambda.context_substitution_slots.clear();
    lambda.context_function.reset();
    lambda.source_name = metadata->local_source_name;
    lambda.namespace_qualifiers = metadata->namespace_qualifiers;
    lambda.signature_parameter_types.clear();
    lambda.discriminator = metadata->discriminator;
    return true;
  }

  string context_fragment;
  vector<abi_mangle::SubstitutionSlot> context_substitution_slots;
  shared_ptr<abi_mangle::FunctionEncoding> context_function;
  if(!build_itanium_lambda_context_fragment_from_type(local_entity_type,
                                                      context_fragment,
                                                      context_substitution_slots,
                                                      &context_function)) {
    return false;
  }
  shared_ptr<FunctionSymbolOptions> context_options;
  if(!itanium_lambda_context_options_from_type(local_entity_type,
                                              context_options)) {
    return false;
  }

  vector<abi_mangle::Type> signature_parameter_types;
  const string source_name =
      local_entity_source_name(*metadata, *context_options);
  if(source_name.empty() &&
     !build_itanium_lambda_signature_parameter_type_ir(
         local_entity_type,
         signature_mangle_ctx,
         signature_parameter_types)) {
    return false;
  }

  abi_mangle::FunctionEncoding::LambdaMetadata & lambda =
      abi_mangle::FunctionEncoding::ensure_lambda_metadata(function);
  lambda.context_fragment = context_fragment;
  lambda.context_substitution_slots = context_substitution_slots;
  lambda.context_function = context_function;
  lambda.source_name = source_name;
  lambda.namespace_qualifiers.clear();
  lambda.signature_parameter_types = signature_parameter_types;
  lambda.discriminator = metadata->discriminator;
  return true;
}

static bool try_build_itanium_abi_lambda_closure_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  const Type::LambdaMangleMetadata * metadata =
      named_type_lambda_mangle_metadata(type);
  if(!metadata) {
    return false;
  }
  if(!lambda_metadata_has_function_context(*metadata)) {
    if(metadata->local_source_name.empty()) {
      return false;
    }
    out = abi_mangle::Type::lambda_closure(
        string(),
        vector<abi_mangle::SubstitutionSlot>(),
        shared_ptr<abi_mangle::FunctionEncoding>(),
        vector<abi_mangle::Type>(),
        metadata->discriminator,
        metadata->local_source_name,
        metadata->namespace_qualifiers);
    attach_context_free_type_ir_substitution(out);
    return true;
  }

  shared_ptr<FunctionSymbolOptions> context_options;
  if(!itanium_lambda_context_options_from_type(type, context_options)) {
    return false;
  }

  string context_fragment;
  vector<abi_mangle::SubstitutionSlot> context_substitution_slots;
  shared_ptr<abi_mangle::FunctionEncoding> context_function;
  if(!build_itanium_lambda_context_fragment_from_type(type,
                                                      context_fragment,
                                                      context_substitution_slots,
                                                      &context_function)) {
    return false;
  }

  vector<abi_mangle::Type> parameter_types;
  const string source_name =
      local_entity_source_name(*metadata, *context_options);
  if(source_name.empty() &&
     !build_itanium_lambda_signature_parameter_type_ir(type,
                                                       mangle_ctx,
                                                       parameter_types)) {
    return false;
  }

  out = abi_mangle::Type::lambda_closure(
      context_fragment,
      context_substitution_slots,
      context_function,
      parameter_types,
      metadata->discriminator,
      source_name,
      vector<string>());
  attach_context_free_type_ir_substitution(out);
  return true;
}

static bool try_build_builtin_type_transform_ast_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  string builtin_name;
  if(!builtin_type_transform_ast_name(node, builtin_name) ||
     node.children.size() != 1 ||
     node.children[0].kind != CppAstKind::type_id) {
    return false;
  }

  abi_mangle::Type argument;
  if(!try_build_type_id_ast_ir(node.children[0], mangle_ctx, argument)) {
    return false;
  }
	  out = abi_mangle::Type::builtin_type_transform(builtin_name,
	                                                 std::move(argument));
  return true;
}

static bool try_build_parameter_clause_type_ast_ir(
    const CppAstNode & clause,
    const abi_mangle::Type & result,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(clause.kind != CppAstKind::parameter_clause) {
    return false;
  }

  vector<abi_mangle::Type> params;
  bool variadic = false;
  for(size_t i = 0; i < clause.children.size(); ++i) {
    const CppAstNode & child = clause.children[i];
    if(child.kind == CppAstKind::ellipsis) {
      variadic = true;
      continue;
    }
    if(child.kind == CppAstKind::parameter_pack) {
      if(params.empty()) {
        return false;
      }
      wrap_pack_expansion_type_ir_if_needed(true, params.back(), true);
      continue;
    }
    if(child.kind != CppAstKind::parameter_declaration) {
      continue;
    }
    abi_mangle::Type param;
    if(!try_build_parameter_declaration_ast_ir(child, mangle_ctx, param)) {
      return false;
    }
    params.push_back(std::move(param));
  }

  out = abi_mangle::Type::function(std::move(result),
                                   std::move(params),
                                   variadic);
  return attach_type_ir_substitution(out);
}

static bool try_build_declarator_ast_type_ir(
    const CppAstNode & declarator,
    const function<bool(abi_mangle::Type &)> & build_base,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(declarator.kind != CppAstKind::declarator &&
     declarator.kind != CppAstKind::abstract_declarator) {
    return build_base(out);
  }

  vector<const CppAstNode *> ptr_operators;
  vector<const CppAstNode *> suffixes;
  const CppAstNode * nested_declarator = nullptr;
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    const CppAstNode & child = declarator.children[i];
    if(child.kind == CppAstKind::ptr_operator) {
      ptr_operators.push_back(&child);
    } else if(child.kind == CppAstKind::array_suffix ||
              child.kind == CppAstKind::parameter_clause) {
      suffixes.push_back(&child);
    } else if(child.kind == CppAstKind::nested_declarator) {
      for(size_t j = 0; j < child.children.size(); ++j) {
        if(child.children[j].kind == CppAstKind::declarator ||
           child.children[j].kind == CppAstKind::abstract_declarator) {
          nested_declarator = &child.children[j];
          break;
        }
      }
    }
  }

  const function<bool(abi_mangle::Type &)> build_pointer_base =
      [&ptr_operators, &build_base](abi_mangle::Type & type) -> bool
      {
        if(!build_base(type)) {
          return false;
        }
        for(vector<const CppAstNode *>::const_reverse_iterator it =
                ptr_operators.rbegin();
            it != ptr_operators.rend();
            ++it) {
          const CppAstNode & op = **it;
          if(op.has_token && op.simple_type == OP_STAR) {
            type = abi_mangle::Type::pointer(std::move(type));
          } else if(op.has_token && op.simple_type == OP_AMP) {
            type = abi_mangle::Type::lvalue_reference(std::move(type));
          } else if(op.has_token && op.simple_type == OP_LAND) {
            type = abi_mangle::Type::rvalue_reference(std::move(type));
          } else {
            return false;
          }
          if(!attach_type_ir_substitution(type)) {
            return false;
          }
        }
        return true;
      };

  function<bool(size_t, abi_mangle::Type &)> build_suffix_type =
      [&](size_t index, abi_mangle::Type & type) -> bool
      {
        if(index == suffixes.size()) {
          return build_pointer_base(type);
        }

        abi_mangle::Type inner;
        if(!build_suffix_type(index + 1, inner)) {
          return false;
        }

        const CppAstNode & suffix = *suffixes[index];
        if(suffix.kind == CppAstKind::array_suffix) {
          string bound;
          string bound_key;
          if(suffix.children.empty()) {
            bound_key = "expr:unknown";
          } else {
            if(!try_build_array_bound_ast_encoding_text(
                   suffix.children[0], bound, mangle_ctx) ||
               !build_array_bound_ast_key(
                   suffix.children[0], mangle_ctx, bound_key)) {
              return false;
            }
          }
          type = abi_mangle::Type::array(std::move(bound),
                                         std::move(bound_key),
                                         std::move(inner));
          return attach_type_ir_substitution(type);
        }

        if(suffix.kind == CppAstKind::parameter_clause) {
          return try_build_parameter_clause_type_ast_ir(
              suffix, inner, mangle_ctx, type);
        }

        return false;
      };

  const function<bool(abi_mangle::Type &)> build_current =
      [&build_suffix_type](abi_mangle::Type & type) -> bool
      {
        return build_suffix_type(0, type);
      };

  if(nested_declarator) {
    return try_build_declarator_ast_type_ir(
        *nested_declarator, build_current, mangle_ctx, out);
  }
  return build_current(out);
}

static bool try_build_type_id_ast_ir(const CppAstNode & node,
                                     const TypeMangleContext * mangle_ctx,
                                     abi_mangle::Type & out)
{
  const bool prefer_syntax =
      ast_node_mentions_direct_template_parameter(node, mangle_ctx) ||
      ast_node_value_has_template_or_scope_syntax(node);
  if(!prefer_syntax &&
     node.semantic_type &&
     try_build_type_ir(node.semantic_type, mangle_ctx, out)) {
    return true;
  }
  if(try_build_builtin_type_transform_ast_ir(node, mangle_ctx, out)) {
    return true;
  }
  if(node.kind != CppAstKind::type_id || node.children.empty()) {
    return false;
  }

  if(!prefer_syntax &&
     node.children.size() == 1 &&
     node.children[0].semantic_type &&
     try_build_type_ir(node.children[0].semantic_type, mangle_ctx, out)) {
    return true;
  }
  if(node.children.size() == 1 &&
     try_build_builtin_type_transform_ast_ir(node.children[0],
                                             mangle_ctx,
                                             out)) {
    return true;
  }
  if(node.children.size() == 1 &&
     (node.children[0].kind == CppAstKind::type_specifier_seq ||
      node.children[0].kind == CppAstKind::decl_specifier_seq) &&
     try_build_type_specifier_seq_ast_ir(node.children[0], mangle_ctx, out)) {
    return true;
  }
  if(node.children[0].kind == CppAstKind::type_specifier_seq ||
     node.children[0].kind == CppAstKind::decl_specifier_seq) {
    abi_mangle::Type specifier_type;
    if(!try_build_type_specifier_seq_ast_ir(node.children[0],
                                            mangle_ctx,
                                            specifier_type)) {
      return false;
    }
    const CppAstNode * abstract = nullptr;
    for(size_t i = 1; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::abstract_declarator) {
        abstract = &node.children[i];
        break;
      }
    }
    if(!abstract) {
      out = specifier_type;
      return true;
    }
    return try_build_declarator_ast_type_ir(
        *abstract,
        [&specifier_type](abi_mangle::Type & type) -> bool
        {
          type = specifier_type;
          return true;
        },
        mangle_ctx,
        out);
  }
  if(prefer_syntax) {
    if(node.semantic_type &&
       try_build_type_ir(node.semantic_type, mangle_ctx, out)) {
      return true;
    }
    if(node.children.size() == 1 &&
       node.children[0].semantic_type &&
       try_build_type_ir(node.children[0].semantic_type, mangle_ctx, out)) {
      return true;
    }
  }
  return false;
}

static bool try_build_parameter_declaration_ast_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(node.kind != CppAstKind::parameter_declaration) {
    return false;
  }

  const CppAstNode * specifiers = nullptr;
  const CppAstNode * declarator = nullptr;
  bool pack_expansion = false;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::decl_specifier_seq ||
       child.kind == CppAstKind::type_specifier_seq) {
      specifiers = &child;
    } else if(child.kind == CppAstKind::declarator ||
              child.kind == CppAstKind::abstract_declarator) {
      declarator = &child;
    } else if(child.kind == CppAstKind::parameter_pack ||
              child.kind == CppAstKind::ellipsis) {
      pack_expansion = true;
    }
  }
  if(!specifiers) {
    return false;
  }

  CppAstNode type_id;
  type_id.kind = CppAstKind::type_id;
  type_id.value = node.value;
  type_id.children.push_back(*specifiers);
  if(declarator) {
    CppAstNode abstract = *declarator;
    abstract.kind = CppAstKind::abstract_declarator;
    type_id.children.push_back(abstract);
  }
  if(!try_build_type_id_ast_ir(type_id, mangle_ctx, out)) {
    return false;
  }
  wrap_pack_expansion_type_ir_if_needed(pack_expansion, out, true);
  return true;
}

static bool ast_node_value_has_template_or_scope_syntax(const CppAstNode & node)
{
  if(node.value.find('<') != string::npos ||
     node.value.find("::") != string::npos) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_value_has_template_or_scope_syntax(node.children[i])) {
      return true;
    }
  }
  return false;
}

static bool try_build_type_specifier_seq_ast_ir(const CppAstNode & node,
                                                const TypeMangleContext * mangle_ctx,
                                                abi_mangle::Type & out)
{
  if(node.kind != CppAstKind::type_specifier_seq &&
     node.kind != CppAstKind::decl_specifier_seq) {
    const bool prefer_syntax =
        ast_node_mentions_direct_template_parameter(node, mangle_ctx) ||
        ast_node_value_has_template_or_scope_syntax(node);
    if(!prefer_syntax &&
       node.semantic_type &&
       try_build_type_ir(node.semantic_type, mangle_ctx, out)) {
      return true;
    }
    if(try_build_builtin_type_transform_ast_ir(node, mangle_ctx, out)) {
      return true;
    }
    if(try_build_template_parameter_type_ast_ir(node,
                                                mangle_ctx,
                                                true,
                                                out)) {
      return true;
    }
    if((node.kind == CppAstKind::decltype_specifier ||
        node.kind == CppAstKind::decl_specifier) &&
       trim_space(node.value).compare(0, 9, "decltype(") == 0 &&
       !node.children.empty()) {
      TypeMangleContext decltype_ctx_storage;
      const TypeMangleContext * decltype_ctx = mangle_ctx;
      if(mangle_ctx) {
        decltype_ctx_storage = *mangle_ctx;
      }
      decltype_ctx_storage.suppress_decltype_callee_template_prefix_substitution = true;
      decltype_ctx = &decltype_ctx_storage;
      abi_mangle::DependentExpression expression;
      if(!try_build_dependent_expression_ir(node.children[0],
                                            decltype_ctx,
                                            expression)) {
        return false;
      }
      out.kind = abi_mangle::Type::TK_DECLTYPE_EXPRESSION;
      out.expression.reset(new abi_mangle::DependentExpression(expression));
      attach_type_ir_substitution(out);
      return true;
    }
    if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
      if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
        if(!qualified->qualifiers.empty() &&
           try_build_qualified_template_id_type_ast_ir(node,
                                                       *template_id,
                                                       *qualified,
                                                       mangle_ctx,
                                                       out)) {
          return true;
        }
      }
      return try_build_template_id_type_ir(*template_id, mangle_ctx, out);
    }
    if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
      if(!qualified->qualifiers.empty() && !qualified->name.empty()) {
        const bool qualified_mentions_direct_template_parameter =
            ast_node_mentions_direct_template_parameter(node, mangle_ctx);
        TypePtr lookup_type = qualified_mentions_direct_template_parameter ?
            TypePtr() :
            lookup_qualified_named_type_for_mangling(*qualified, mangle_ctx);
        if(lookup_type && try_build_type_ir(lookup_type, mangle_ctx, out)) {
          return true;
        }
        if(try_build_lexically_prefixed_qualified_type_ast_ir(node,
                                                              *qualified,
                                                              mangle_ctx,
                                                              out)) {
          return true;
        }
        QualifiedName effective = *qualified;
        const string first_qualifier =
            semantic_utils::strip_trailing_top_level_template_arguments(
                trim_space(effective.qualifiers[0]));
        QualifiedName lookup_prefix;
        bool applied_lexical_prefix = false;
        if(!syntax_name_matches_template_parameter(first_qualifier, mangle_ctx) &&
           qualified_type_text_lexical_lookup_prefix(effective,
                                                     mangle_ctx,
                                                     lookup_prefix)) {
          if(!prepend_qualified_prefix(effective, lookup_prefix)) {
            return false;
          }
          applied_lexical_prefix = true;
        }
        vector<TemplateIdSyntax> qualifier_template_id_syntaxes;
        const vector<TemplateIdSyntax> * qualifier_template_id_syntaxes_ptr =
            &node.qualifier_template_id_syntaxes.as_vector();
        if(applied_lexical_prefix) {
          qualifier_template_id_syntaxes.resize(effective.qualifiers.size());
          const size_t prefix_count =
              effective.qualifiers.size() - qualified->qualifiers.size();
          for(size_t i = 0;
              i < node.qualifier_template_id_syntaxes.size();
              ++i) {
            if(i + prefix_count < qualifier_template_id_syntaxes.size()) {
              qualifier_template_id_syntaxes[i + prefix_count] =
                  node.qualifier_template_id_syntaxes[i];
            }
          }
          qualifier_template_id_syntaxes_ptr = &qualifier_template_id_syntaxes;
        }
        abi_mangle::Type current;
        if(!try_build_qualified_owner_type_ir(
               effective,
               qualifier_template_id_syntaxes_ptr,
               mangle_ctx,
               current)) {
          return false;
        }
        string member_substitution_text = node.value;
        string semantic_key;
        static const char name_key_prefix[] = "name:";
        if(node.semantic_type &&
           build_type_substitution_key(node.semantic_type,
                                       mangle_ctx,
                                       semantic_key) &&
           semantic_key.compare(0, sizeof(name_key_prefix) - 1, name_key_prefix) == 0) {
          member_substitution_text =
              semantic_key.substr(sizeof(name_key_prefix) - 1);
        }
        out = abi_mangle::Type::member_named_type(std::move(current),
	                                             effective.name,
	                                             string());
        attach_member_type_ir_substitution(out, member_substitution_text);
        return true;
      }
    }
    if(!prefer_syntax &&
       !node.value.empty() &&
       try_build_direct_type_syntax_text_ir(node.value, mangle_ctx, out)) {
      return true;
    }
    if(prefer_syntax &&
       node.semantic_type &&
       try_build_type_ir(node.semantic_type, mangle_ctx, out)) {
      return true;
    }
    return false;
  }

  const bool prefer_syntax =
      ast_node_mentions_direct_template_parameter(node, mangle_ctx) ||
      ast_node_value_has_template_or_scope_syntax(node);
  if(!prefer_syntax &&
     node.semantic_type &&
     try_build_type_ir(node.semantic_type, mangle_ctx, out)) {
    return true;
  }
  if(try_build_builtin_type_transform_ast_ir(node, mangle_ctx, out)) {
    return true;
  }
  if(try_build_fundamental_type_specifier_seq_ast_ir(node, mangle_ctx, out)) {
    return true;
  }

  const CppAstNode * type_node = nullptr;
  bool cv_const = false;
  bool cv_volatile = false;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.value == "const") {
      cv_const = true;
      continue;
    }
    if(child.value == "volatile") {
      cv_volatile = true;
      continue;
    }
    if(child.value == "typename" ||
       child.value == "class" ||
       child.value == "struct") {
      continue;
    }
    type_node = &child;
  }

  if(!type_node ||
     !try_build_type_specifier_seq_ast_ir(*type_node, mangle_ctx, out)) {
    if(prefer_syntax &&
       node.semantic_type &&
       try_build_type_ir(node.semantic_type, mangle_ctx, out)) {
      return true;
    }
    return false;
  }
  if(cv_const || cv_volatile) {
    out = abi_mangle::Type::cv(cv_const, cv_volatile, std::move(out));
    return attach_context_free_type_ir_substitution(out);
  }
  return true;
}

static bool try_build_literal_non_type_template_argument_ir(
    const string & text,
    abi_mangle::Type::ClassTemplateArgument & out)
{
  const string trimmed = trim_space(text);
  if(trimmed.empty()) {
    return false;
  }
  if(is_signed_decimal_integer_text(trimmed)) {
    out = abi_mangle::Type::ClassTemplateArgument::
        untyped_integral_value_arg(parse_signed_decimal_integer_value(trimmed));
    return true;
  }
  if(trimmed == "false" || trimmed == "true") {
    out = abi_mangle::Type::ClassTemplateArgument::
        integral_value_arg(abi_mangle::Type::builtin("b"),
                           trimmed == "true" ? 1 : 0);
    return true;
  }
  return false;
}

static bool try_build_template_argument_syntax_ir(
    const TemplateArgumentSyntax & syntax,
    const TemplateParameterInfo * parameter,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type::ClassTemplateArgument & out)
{
  const bool known_non_type_parameter =
      parameter && parameter->kind == TemplateParameterInfo::TP_NON_TYPE;
  const bool unknown_expression_argument =
      !parameter &&
      !syntax.type_id &&
      !syntax.resolved_type &&
      !syntax.template_id;
  if(known_non_type_parameter &&
     syntax.expression &&
     try_build_literal_non_type_template_argument_ir(syntax.expression->value,
                                                     out)) {
    return true;
  }
  const string non_type_expression_text =
      non_type_template_argument_expression_text_for_ir(syntax);
  if(known_non_type_parameter &&
     !non_type_expression_text.empty() &&
     !syntax.type_id &&
     !syntax.template_id &&
     try_build_owner_non_type_template_argument_ir(non_type_expression_text,
                                                   syntax.pack_expansion,
                                                   mangle_ctx,
                                                   out)) {
    return true;
  }
  if((known_non_type_parameter || unknown_expression_argument) &&
     syntax.expression) {
    abi_mangle::DependentExpression expression;
    if(try_build_dependent_expression_ir(*syntax.expression,
                                         mangle_ctx,
                                         expression)) {
      if(syntax.pack_expansion) {
        expression =
            abi_mangle::DependentExpression::pack_expansion(
                std::move(expression));
      }
      out = abi_mangle::Type::ClassTemplateArgument::
          dependent_expression_arg(std::move(expression));
      return true;
    }
  }
  if(known_non_type_parameter &&
     !non_type_expression_text.empty() &&
     try_build_literal_non_type_template_argument_ir(non_type_expression_text,
                                                     out)) {
    return true;
  }
  if(known_non_type_parameter) {
    abi_mangle::DependentExpression expression;
    if(!non_type_expression_text.empty() &&
       try_build_owner_non_type_template_argument_ir(non_type_expression_text,
                                                     syntax.pack_expansion,
                                                     mangle_ctx,
                                                     out)) {
      return true;
    }
    if(!non_type_expression_text.empty() &&
       try_build_template_parameter_value_expression_ir(non_type_expression_text,
                                                        mangle_ctx,
                                                        expression)) {
      if(syntax.pack_expansion) {
        expression =
            abi_mangle::DependentExpression::pack_expansion(
                std::move(expression));
      }
      out = abi_mangle::Type::ClassTemplateArgument::
          dependent_expression_arg(std::move(expression));
      return true;
    }
    return false;
  }

  abi_mangle::Type type;
  if(syntax.pack_expansion &&
     !syntax.text.empty() &&
     try_build_concrete_owner_pack_expansion_type_ir(syntax.text,
                                                     mangle_ctx,
                                                     type)) {
    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }
  const bool using_source_template_parameter_argument =
      mangle_ctx &&
      mangle_ctx->prefer_source_template_parameter_expression_arguments &&
      !syntax.source_text.empty();
  const string source_or_text =
      trim_space(using_source_template_parameter_argument ?
                     syntax.source_text :
                     syntax.text);
  if(!known_non_type_parameter &&
     mangle_ctx &&
     mangle_ctx->prefer_source_template_parameter_expression_arguments &&
     syntax.source_type_id) {
    CppAstNode source_type_id = clone_ast_node_for_mangling(*syntax.source_type_id);
    clear_default_type_argument_semantics_for_mangling(source_type_id);
    if(try_build_type_id_ast_ir(source_type_id, mangle_ctx, type)) {
      if(!(syntax.pack_expansion &&
           pack_expansion_text_resolves_to_concrete_owner_pack(source_or_text,
                                                               mangle_ctx))) {
        wrap_pack_expansion_type_ir_if_needed(syntax.pack_expansion, type);
      }
      out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
      return true;
    }
  }
  if(!known_non_type_parameter &&
     !source_or_text.empty() &&
     try_build_template_parameter_type_text_ir(source_or_text,
                                               mangle_ctx,
                                               true,
                                               type)) {
    if(!(syntax.pack_expansion &&
         pack_expansion_text_resolves_to_concrete_owner_pack(source_or_text,
                                                             mangle_ctx))) {
      wrap_pack_expansion_type_ir_if_needed(syntax.pack_expansion, type);
    }
    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }
  if(!known_non_type_parameter &&
     mangle_ctx &&
     mangle_ctx->prefer_source_template_parameter_expression_arguments &&
     syntax.type_id &&
     ast_node_mentions_direct_template_parameter(*syntax.type_id, mangle_ctx) &&
     try_build_type_id_ast_ir(*syntax.type_id, mangle_ctx, type)) {
    if(!(syntax.pack_expansion &&
         pack_expansion_text_resolves_to_concrete_owner_pack(source_or_text,
                                                             mangle_ctx))) {
      wrap_pack_expansion_type_ir_if_needed(syntax.pack_expansion, type);
    }
    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }
  const bool resolved_type_is_dependent =
      syntax.resolved_type && type_has_dependent_mangle_state(syntax.resolved_type);
  const bool has_structured_type_syntax = syntax.type_id || syntax.template_id;
  if(resolved_type_is_dependent &&
     syntax.type_id &&
     try_build_type_id_ast_ir(*syntax.type_id, mangle_ctx, type)) {
    if(!(syntax.pack_expansion &&
         pack_expansion_text_resolves_to_concrete_owner_pack(syntax.text,
                                                             mangle_ctx))) {
      wrap_pack_expansion_type_ir_if_needed(syntax.pack_expansion, type);
    }
    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }
  if(resolved_type_is_dependent &&
     syntax.template_id &&
     try_build_template_id_type_ir(*syntax.template_id, mangle_ctx, type)) {
    wrap_pack_expansion_type_ir_if_needed(syntax.pack_expansion, type);
    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }
  if(syntax.resolved_type &&
     !(resolved_type_is_dependent && has_structured_type_syntax) &&
     try_build_type_ir(syntax.resolved_type, mangle_ctx, type)) {
    wrap_pack_expansion_type_ir_if_needed(syntax.pack_expansion, type);
    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }
  if(syntax.type_id &&
     try_build_type_id_ast_ir(*syntax.type_id, mangle_ctx, type)) {
    if(!(syntax.pack_expansion &&
         pack_expansion_text_resolves_to_concrete_owner_pack(syntax.text,
                                                             mangle_ctx))) {
      wrap_pack_expansion_type_ir_if_needed(syntax.pack_expansion, type);
    }
    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }
  if(syntax.type_id &&
     !resolved_type_is_dependent &&
     !syntax.text.empty() &&
     try_build_direct_type_syntax_text_ir(syntax.text, mangle_ctx, type)) {
    if(!(syntax.pack_expansion &&
         pack_expansion_text_resolves_to_concrete_owner_pack(syntax.text,
                                                             mangle_ctx))) {
      wrap_pack_expansion_type_ir_if_needed(syntax.pack_expansion, type);
    }
    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }
  if(!known_non_type_parameter &&
     !has_structured_type_syntax &&
     !syntax.text.empty() &&
     try_build_direct_type_syntax_text_ir(syntax.text, mangle_ctx, type)) {
    if(!(syntax.pack_expansion &&
         pack_expansion_text_resolves_to_concrete_owner_pack(syntax.text,
                                                             mangle_ctx))) {
      wrap_pack_expansion_type_ir_if_needed(syntax.pack_expansion, type);
    }
    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }
  if(!syntax.text.empty() &&
     !syntax.type_id &&
     !syntax.template_id &&
     try_build_resolved_type_argument_text_ir(syntax.text,
                                              mangle_ctx,
                                              type)) {
    if(!(syntax.pack_expansion &&
         pack_expansion_text_resolves_to_concrete_owner_pack(syntax.text,
                                                             mangle_ctx))) {
      wrap_pack_expansion_type_ir_if_needed(syntax.pack_expansion, type);
    }
    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }
  if(syntax.template_id &&
     try_build_template_id_type_ir(*syntax.template_id, mangle_ctx, type)) {
    wrap_pack_expansion_type_ir_if_needed(syntax.pack_expansion, type);
    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }
  if(!parameter &&
     !syntax.text.empty() &&
     !syntax.type_id &&
     !syntax.template_id) {
    abi_mangle::DependentExpression expression;
    if(try_build_template_parameter_value_expression_ir(syntax.text,
                                                        mangle_ctx,
                                                        expression)) {
      if(syntax.pack_expansion) {
        expression =
            abi_mangle::DependentExpression::pack_expansion(
                std::move(expression));
      }
      out = abi_mangle::Type::ClassTemplateArgument::
          dependent_expression_arg(std::move(expression));
      return true;
    }
  }
  return false;
}

static bool dependent_binary_operator_mangle_code(const string & op,
                                                  string & out)
{
  if(op == "*") {
    out = "ml";
    return true;
  }
  if(op == "/") {
    out = "dv";
    return true;
  }
  if(op == "%") {
    out = "rm";
    return true;
  }
  if(op == "+") {
    out = "pl";
    return true;
  }
  if(op == "-") {
    out = "mi";
    return true;
  }
  if(op == "<<") {
    out = "ls";
    return true;
  }
  if(op == ">>") {
    out = "rs";
    return true;
  }
  if(op == "<") {
    out = "lt";
    return true;
  }
  if(op == ">") {
    out = "gt";
    return true;
  }
  if(op == "<=") {
    out = "le";
    return true;
  }
  if(op == ">=") {
    out = "ge";
    return true;
  }
  if(op == "&&") {
    out = "aa";
    return true;
  }
  if(op == "||") {
    out = "oo";
    return true;
  }
  if(op == "==") {
    out = "eq";
    return true;
  }
  if(op == "!=") {
    out = "ne";
    return true;
  }
  if(op == "&") {
    out = "an";
    return true;
  }
  if(op == "^") {
    out = "eo";
    return true;
  }
  if(op == "|") {
    out = "or";
    return true;
  }
  if(op == ",") {
    out = "cm";
    return true;
  }
  return false;
}

static bool try_build_constant_sizeof_type_id_expression_ir(
    const CppAstNode & type_id,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::DependentExpression & out)
{
  if(type_id.kind != CppAstKind::type_id ||
     ast_node_mentions_direct_template_parameter(type_id, mangle_ctx)) {
    return false;
  }

  TypePtr type = type_id.semantic_type;
  if(!type &&
     type_id.children.size() == 1 &&
     type_id.children[0].semantic_type) {
    type = type_id.children[0].semantic_type;
  }
  if(!type) {
    return false;
  }

  TypePtr sizeof_type = remove_reference_type(type);
  if(!sizeof_type) {
    sizeof_type = type;
  }
  sizeof_type = strip_top_level_cv(sizeof_type);
  if(!sizeof_type || type_has_dependent_mangle_state(sizeof_type)) {
    return false;
  }

  out = abi_mangle::DependentExpression::typed_integral_value(
      abi_mangle::Type::builtin("m"),
      static_cast<long long>(type_size(sizeof_type)));
  return true;
}

static bool builtin_mangle_code_type_size(const string & code, size_t & out)
{
  if(code == "a") { out = type_to_size(FT_SIGNED_CHAR); return true; }
  if(code == "s") { out = type_to_size(FT_SHORT_INT); return true; }
  if(code == "i") { out = type_to_size(FT_INT); return true; }
  if(code == "l") { out = type_to_size(FT_LONG_INT); return true; }
  if(code == "x") { out = type_to_size(FT_LONG_LONG_INT); return true; }
  if(code == "n") { out = type_to_size(FT_INT128); return true; }
  if(code == "h") { out = type_to_size(FT_UNSIGNED_CHAR); return true; }
  if(code == "t") { out = type_to_size(FT_UNSIGNED_SHORT_INT); return true; }
  if(code == "j") { out = type_to_size(FT_UNSIGNED_INT); return true; }
  if(code == "m") { out = type_to_size(FT_UNSIGNED_LONG_INT); return true; }
  if(code == "y") { out = type_to_size(FT_UNSIGNED_LONG_LONG_INT); return true; }
  if(code == "o") { out = type_to_size(FT_UINT128); return true; }
  if(code == "w") { out = type_to_size(FT_WCHAR_T); return true; }
  if(code == "c") { out = type_to_size(FT_CHAR); return true; }
  if(code == "Ds") { out = type_to_size(FT_CHAR16_T); return true; }
  if(code == "Di") { out = type_to_size(FT_CHAR32_T); return true; }
  if(code == "b") { out = type_to_size(FT_BOOL); return true; }
  if(code == "f") { out = type_to_size(FT_FLOAT); return true; }
  if(code == "d") { out = type_to_size(FT_DOUBLE); return true; }
  if(code == "e") { out = type_to_size(FT_LONG_DOUBLE); return true; }
  if(code == "Dn") { out = type_to_size(FT_NULLPTR_T); return true; }
  return false;
}

static bool try_build_constant_sizeof_ir_type_expression(
    const abi_mangle::Type & type,
    abi_mangle::DependentExpression & out)
{
  if(abi_mangle::type_contains_template_parameter_ref(type)) {
    return false;
  }

  size_t size = 0;
  switch(type.kind) {
  case abi_mangle::Type::TK_BUILTIN: {
    string code = type.builtin_code;
    if(!builtin_mangle_code_type_size(code, size)) {
      return false;
    }
    break;
  }
  case abi_mangle::Type::TK_POINTER:
  case abi_mangle::Type::TK_MEMBER_POINTER:
  case abi_mangle::Type::TK_LVALUE_REFERENCE:
  case abi_mangle::Type::TK_RVALUE_REFERENCE:
    size = 8;
    break;
  default:
    return false;
  }

  out = abi_mangle::DependentExpression::typed_integral_value(
      abi_mangle::Type::builtin("m"),
      static_cast<long long>(size));
  return true;
}

static bool build_dependent_template_id_expression_arguments_ir(
    const TemplateIdSyntax & template_id,
    const TypeMangleContext * mangle_ctx,
    vector<abi_mangle::TemplateArgument> & out)
{
  vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
  dependent_arguments.reserve(template_id.argument_syntaxes.size());
  for(size_t i = 0; i < template_id.argument_syntaxes.size(); ++i) {
    DependentAliasTemplateArgumentSyntax argument;
    argument.syntax = template_id.argument_syntaxes[i];
    argument.text = argument.syntax.text;
    if(argument.text.empty() && i < template_id.arguments.size()) {
      argument.text = template_id.arguments[i];
      argument.syntax.text = argument.text;
    }
    if(argument.syntax.type_id && argument.syntax.type_id->semantic_type) {
      argument.type = argument.syntax.type_id->semantic_type;
    } else if(argument.syntax.resolved_type) {
      argument.type = argument.syntax.resolved_type;
    }
    dependent_arguments.push_back(std::move(argument));
  }

  vector<abi_mangle::Type::ClassTemplateArgument> class_arguments;
  TypeMangleContext expression_argument_ctx_storage;
  const TypeMangleContext * expression_argument_ctx = mangle_ctx;
  if(mangle_ctx) {
    expression_argument_ctx_storage = *mangle_ctx;
    expression_argument_ctx_storage
        .prefer_source_template_parameter_expression_arguments = true;
    expression_argument_ctx = &expression_argument_ctx_storage;
  }
  if(!build_dependent_template_arguments_ir(dependent_arguments,
                                            nullptr,
                                            expression_argument_ctx,
                                            class_arguments)) {
    return false;
  }
  out.clear();
  out.reserve(class_arguments.size());
  for(size_t i = 0; i < class_arguments.size(); ++i) {
    abi_mangle::TemplateArgument argument;
    if(!class_template_argument_ir_to_template_argument_ir(
           class_arguments[i],
           argument)) {
      return false;
    }
    out.push_back(std::move(argument));
  }
  return true;
}

static bool scope_declares_expression_name(
    const semantic_model::Scope & scope,
    const string & name)
{
  if(name.empty()) {
    return false;
  }
  if(scope.values.find(name) != scope.values.end() ||
     scope.function_sets.find(name) != scope.function_sets.end() ||
     scope.function_templates.find(name) != scope.function_templates.end()) {
    return true;
  }
  return scope.class_info &&
         scope.class_info->methods.find(name) != scope.class_info->methods.end();
}

static bool lookup_current_class_member_expression_owner(
    const string & name,
    const TypeMangleContext * mangle_ctx,
    const semantic_model::ClassInfo *& out)
{
  out = nullptr;
  if(name.empty() || !mangle_ctx || !mangle_ctx->lookup_scope) {
    return false;
  }

  for(const semantic_model::Scope * current = mangle_ctx->lookup_scope;
      current;
      current = current->parent) {
    const bool declares_name = scope_declares_expression_name(*current, name);
    if(current->class_info &&
       current->class_info->member_scope.get() == current &&
       declares_name) {
      out = current->class_info;
      return out && out->type;
    }
    if(declares_name) {
      return false;
    }
  }
  return false;
}

static bool try_build_current_class_member_expression_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::DependentExpression & out)
{
  if(node.kind != CppAstKind::id_expression &&
     node.kind != CppAstKind::identifier) {
    return false;
  }
  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
    return false;
  }

  const TemplateIdSyntax * template_id = cppast_template_id_syntax(node);
  if(template_id &&
     (template_id->name.rooted || !template_id->name.qualifiers.empty())) {
    return false;
  }

  const string member_name =
      template_id ? trim_space(strip_leading_template_disambiguator(
                        template_id->name.name)) :
                    trim_space(node.value);
  const semantic_model::ClassInfo * owner_class = nullptr;
  if(member_name.empty() ||
     !lookup_current_class_member_expression_owner(member_name,
                                                   mangle_ctx,
                                                   owner_class)) {
    return false;
  }

  abi_mangle::Type owner;
  if(!try_build_type_ir(owner_class->type, mangle_ctx, owner)) {
    return false;
  }
  bool close_owner = !abi_mangle::type_contains_template_parameter_ref(owner);
  abi_mangle::DependentExpression member =
      abi_mangle::DependentExpression::member(std::move(owner),
                                              close_owner,
                                              member_name);
  if(member.owner_type && member.owner_type->name_owner) {
    member.close_member_owner = false;
  }
  if(template_id &&
     !build_dependent_template_id_expression_arguments_ir(*template_id,
                                                          mangle_ctx,
                                                          member.template_arguments)) {
    return false;
  }
  out = std::move(member);
  return true;
}

static bool try_build_dependent_expression_ir(
    const CppAstNode & node,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::DependentExpression & out)
{
  if(node.kind == CppAstKind::parenthesized_expression &&
     node.children.size() == 1) {
    return try_build_dependent_expression_ir(node.children[0], mangle_ctx, out);
  }
  if(node.kind == CppAstKind::id_expression ||
     node.kind == CppAstKind::identifier) {
    size_t function_parameter_index = 0;
    if(function_parameter_index_for_name(node.value,
                                         mangle_ctx,
                                         function_parameter_index)) {
      out = abi_mangle::DependentExpression::function_parameter(
          function_parameter_index);
      return true;
    }
  }
  if((node.kind == CppAstKind::id_expression ||
      node.kind == CppAstKind::identifier) &&
     try_build_template_parameter_value_expression_ir(node.value,
                                                      mangle_ctx,
                                                      out)) {
    return true;
  }
  if(node.kind == CppAstKind::literal &&
     is_signed_decimal_integer_text(node.value)) {
    out = abi_mangle::DependentExpression::literal(node.value);
    return true;
  }
  if(node.kind == CppAstKind::sizeof_pack_expression &&
     node.children.size() == 1 &&
     node.children[0].kind == CppAstKind::identifier &&
     try_build_sizeof_pack_expression_ir(node.children[0].value,
                                         mangle_ctx,
                                         out)) {
    return true;
  }
  if(node.kind == CppAstKind::sizeof_expression &&
     node.children.size() == 1 &&
     node.children[0].kind == CppAstKind::type_id) {
    if(try_build_constant_sizeof_type_id_expression_ir(node.children[0],
                                                       mangle_ctx,
                                                       out)) {
      return true;
    }
    abi_mangle::Type type;
    if(!try_build_type_id_ast_ir(node.children[0], mangle_ctx, type)) {
      return false;
    }
    if(try_build_constant_sizeof_ir_type_expression(type, out)) {
      return true;
    }
    out = abi_mangle::DependentExpression::sizeof_type(type);
    return true;
  }
  if(node.kind == CppAstKind::pack_expansion_expression &&
     node.children.size() == 1) {
    abi_mangle::DependentExpression inner;
    if(!try_build_dependent_expression_ir(node.children[0],
                                          mangle_ctx,
                                          inner)) {
      return false;
    }
    out = abi_mangle::DependentExpression::pack_expansion(inner);
    return true;
  }
  if(node.kind == CppAstKind::type_trait_expression &&
     node.simple_type == KW_NOEXCEPT &&
     node.children.size() == 1) {
    abi_mangle::DependentExpression inner;
    if(!try_build_dependent_expression_ir(node.children[0],
                                          mangle_ctx,
                                          inner)) {
      return false;
    }
    out = abi_mangle::DependentExpression::unary("nx", std::move(inner));
    return true;
  }
  if(node.kind == CppAstKind::type_trait_expression &&
     !node.value.empty() &&
     node.value.compare(0, 2, "__") == 0) {
    vector<abi_mangle::Type> type_arguments;
    type_arguments.reserve(node.children.size());
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind != CppAstKind::type_id) {
        return false;
      }
      abi_mangle::Type type_argument;
      if(!try_build_type_id_ast_ir(node.children[i],
                                   mangle_ctx,
                                   type_argument)) {
        return false;
      }
      type_arguments.push_back(type_argument);
    }
    out = abi_mangle::DependentExpression::type_trait(node.value,
                                                             type_arguments);
    return true;
  }
  if(node.kind == CppAstKind::cast_expression &&
     node.children.size() == 2 &&
     node.value == "static_cast") {
    abi_mangle::Type target_type;
    abi_mangle::DependentExpression argument;
    if(!try_build_cast_target_type_id_ast_ir(node.children[0],
                                             mangle_ctx,
                                             target_type) ||
       !try_build_dependent_expression_ir(node.children[1],
                                          mangle_ctx,
                                          argument)) {
      return false;
    }
    out = abi_mangle::DependentExpression::cast("sc",
                                                       target_type,
                                                       argument);
    return true;
  }
  if(node.kind == CppAstKind::unary_expression && node.children.size() == 1) {
    string op_code;
    if(node.value == "*") {
      op_code = "de";
    } else if(node.value == "&") {
      op_code = "ad";
    } else if(node.value == "+") {
      op_code = "ps";
    } else if(node.value == "-") {
      op_code = "ng";
    } else if(node.value == "!") {
      op_code = "nt";
    } else if(node.value == "~") {
      op_code = "co";
    }
    abi_mangle::DependentExpression inner;
    if(op_code.empty() ||
       !try_build_dependent_expression_ir(node.children[0], mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::DependentExpression::unary(op_code, inner);
    return true;
  }
  if(node.kind == CppAstKind::binary_expression &&
     node.children.size() == 2) {
    string op_code;
    abi_mangle::DependentExpression left;
    abi_mangle::DependentExpression right;
    if(!dependent_binary_operator_mangle_code(node.value, op_code)) {
      return false;
    }
    if(!try_build_dependent_expression_ir(node.children[0],
                                          mangle_ctx,
                                          left)) {
      return false;
    }
    if(!try_build_dependent_expression_ir(node.children[1],
                                          mangle_ctx,
                                          right)) {
      return false;
    }
    out = abi_mangle::DependentExpression::binary(op_code, left, right);
    return true;
  }
  if(node.kind == CppAstKind::conditional_expression &&
     node.children.size() == 3) {
    abi_mangle::DependentExpression condition;
    abi_mangle::DependentExpression true_expr;
    abi_mangle::DependentExpression false_expr;
    if(!try_build_dependent_expression_ir(node.children[0],
                                          mangle_ctx,
                                          condition) ||
       !try_build_dependent_expression_ir(node.children[1],
                                          mangle_ctx,
                                          true_expr) ||
       !try_build_dependent_expression_ir(node.children[2],
                                          mangle_ctx,
                                          false_expr)) {
      return false;
    }
    out = abi_mangle::DependentExpression::conditional(condition,
                                                              true_expr,
                                                              false_expr);
    return true;
  }
  if(node.kind == CppAstKind::member_expression &&
     node.children.size() == 2 &&
     (node.value == "." || node.value == "->")) {
    abi_mangle::DependentExpression object;
    if(!try_build_dependent_expression_ir(node.children[0],
                                          mangle_ctx,
                                          object)) {
      return false;
    }
    const CppAstNode & member = node.children[1];
    if(member.kind != CppAstKind::identifier || member.value.empty()) {
      return false;
    }
    string member_name = member.value;
    vector<abi_mangle::TemplateArgument> template_arguments;
    if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(member)) {
      member_name = trim_space(template_id->name.name);
      if(member_name.empty() || !template_id->name.qualifiers.empty()) {
        return false;
      }
      vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
      dependent_arguments.reserve(template_id->argument_syntaxes.size());
      for(size_t i = 0; i < template_id->argument_syntaxes.size(); ++i) {
        DependentAliasTemplateArgumentSyntax argument;
        argument.syntax = template_id->argument_syntaxes[i];
        argument.text = argument.syntax.text;
        if(argument.text.empty() && i < template_id->arguments.size()) {
          argument.text = template_id->arguments[i];
          argument.syntax.text = argument.text;
        }
        if(argument.syntax.type_id && argument.syntax.type_id->semantic_type) {
          argument.type = argument.syntax.type_id->semantic_type;
        } else if(argument.syntax.resolved_type) {
          argument.type = argument.syntax.resolved_type;
        }
        dependent_arguments.push_back(std::move(argument));
      }
      vector<abi_mangle::Type::ClassTemplateArgument> class_arguments;
      TypeMangleContext expression_argument_ctx_storage;
      const TypeMangleContext * expression_argument_ctx = mangle_ctx;
      if(mangle_ctx) {
        expression_argument_ctx_storage = *mangle_ctx;
        expression_argument_ctx_storage
            .prefer_source_template_parameter_expression_arguments = true;
        expression_argument_ctx = &expression_argument_ctx_storage;
      }
      if(!build_dependent_template_arguments_ir(dependent_arguments,
                                                nullptr,
                                                expression_argument_ctx,
                                                class_arguments)) {
        return false;
      }
      template_arguments.reserve(class_arguments.size());
      for(size_t i = 0; i < class_arguments.size(); ++i) {
        abi_mangle::TemplateArgument argument;
        if(!class_template_argument_ir_to_template_argument_ir(
               class_arguments[i],
               argument)) {
          return false;
        }
        template_arguments.push_back(std::move(argument));
      }
    }
    out = abi_mangle::DependentExpression::object_member(
        node.value == "->" ? "pt" : "dt",
        object,
        member_name,
        template_arguments);
    return true;
  }
  if(node.kind == CppAstKind::call_expression && !node.children.empty()) {
    if(const CppAstNode * conversion_type_id =
           cppast_conversion_type_id_syntax(node)) {
      abi_mangle::Type conversion_type;
      if(!try_build_type_id_ast_ir(*conversion_type_id,
                                   mangle_ctx,
                                   conversion_type)) {
        return false;
      }
      vector<abi_mangle::DependentExpression> arguments;
      if(node.children.size() >= 2 &&
         (node.children[1].kind == CppAstKind::argument_list ||
          node.children[1].kind == CppAstKind::paren_argument_list)) {
        const CppAstNode & argument_list = node.children[1];
        arguments.reserve(argument_list.children.size());
        for(size_t i = 0; i < argument_list.children.size(); ++i) {
          abi_mangle::DependentExpression argument;
          if(!try_build_dependent_expression_ir(argument_list.children[i],
                                                mangle_ctx,
                                                argument)) {
            return false;
          }
          arguments.push_back(std::move(argument));
        }
      }
      out = abi_mangle::DependentExpression::conversion(conversion_type,
                                                               arguments);
      return true;
    }

    abi_mangle::DependentExpression callee;
    if(!try_build_dependent_expression_ir(node.children[0], mangle_ctx, callee)) {
      return false;
    }
    if(mangle_ctx &&
       mangle_ctx->suppress_decltype_callee_template_prefix_substitution) {
      callee.suppress_template_prefix_substitution = true;
    } else {
      callee.suppress_template_prefix_substitution = false;
    }
    vector<abi_mangle::DependentExpression> arguments;
    if(node.children.size() >= 2 &&
       (node.children[1].kind == CppAstKind::argument_list ||
        node.children[1].kind == CppAstKind::paren_argument_list)) {
      const CppAstNode & argument_list = node.children[1];
      arguments.reserve(argument_list.children.size());
      for(size_t i = 0; i < argument_list.children.size(); ++i) {
        abi_mangle::DependentExpression argument;
        if(!try_build_dependent_expression_ir(argument_list.children[i],
                                              mangle_ctx,
                                              argument)) {
          return false;
        }
        arguments.push_back(std::move(argument));
      }
    }
    out = abi_mangle::DependentExpression::call(callee, arguments);
    return true;
  }
  if(node.kind == CppAstKind::id_expression ||
     node.kind == CppAstKind::identifier) {
    if(!ast_node_mentions_direct_template_parameter(node, mangle_ctx) &&
       try_build_resolved_constant_member_expression_ir(node,
                                                        mangle_ctx,
                                                        out)) {
      return true;
    }
    const QualifiedName * qualified = cppast_qualified_name_syntax(node);
    if(qualified && !qualified->qualifiers.empty() && !qualified->name.empty()) {
      abi_mangle::Type owner;
      bool close_owner = false;
      const TemplateIdSyntax * qualifier_template_id = nullptr;
      for(size_t i = qualified->qualifiers.size();
          i > 0 && i <= node.qualifier_template_id_syntaxes.size();
          --i) {
        const TemplateIdSyntax & candidate =
            node.qualifier_template_id_syntaxes[i - 1];
        if(!candidate.name.name.empty()) {
          qualifier_template_id = &candidate;
          break;
        }
      }
      TypeMangleContext expression_owner_ctx_storage;
      const TypeMangleContext * expression_owner_ctx = mangle_ctx;
      if(mangle_ctx) {
        expression_owner_ctx_storage = *mangle_ctx;
        expression_owner_ctx_storage
            .prefer_source_template_name_prefixes_in_expressions = true;
        expression_owner_ctx = &expression_owner_ctx_storage;
      }
      if(!try_build_qualified_owner_type_ir(
             *qualified,
             &node.qualifier_template_id_syntaxes.as_vector(),
             expression_owner_ctx,
             owner,
             &close_owner,
             false,
             true,
             mangle_ctx &&
             mangle_ctx->suppress_decltype_callee_template_prefix_substitution)) {
        return false;
      }
      if(!close_owner &&
         !abi_mangle::type_contains_template_parameter_ref(owner)) {
        close_owner = true;
      }

      const TemplateIdSyntax * member_template_id =
          cppast_template_id_syntax(node);
      const string member_name =
          member_template_id && !trim_space(member_template_id->name.name).empty() ?
              trim_space(member_template_id->name.name) :
              qualified->name;
      abi_mangle::DependentExpression member =
          abi_mangle::DependentExpression::member(
              owner,
              close_owner,
              member_name);
      member.suppress_member_owner_prefix =
          qualifier_template_id &&
          !qualifier_template_id->name.rooted &&
          qualifier_template_id->name.qualifiers.empty() &&
          owner.name_owner &&
          !abi_mangle::type_needs_member_expression_template_name_registration(
              owner);
      if(owner.name_owner && !member.suppress_member_owner_prefix) {
        member.close_member_owner = false;
      }
      if(const TemplateIdSyntax * template_id = member_template_id) {
        vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
        dependent_arguments.reserve(template_id->argument_syntaxes.size());
        for(size_t i = 0; i < template_id->argument_syntaxes.size(); ++i) {
          DependentAliasTemplateArgumentSyntax argument;
          argument.syntax = template_id->argument_syntaxes[i];
          argument.text = argument.syntax.text;
          if(argument.text.empty() && i < template_id->arguments.size()) {
            argument.text = template_id->arguments[i];
            argument.syntax.text = argument.text;
          }
          if(argument.syntax.type_id && argument.syntax.type_id->semantic_type) {
            argument.type = argument.syntax.type_id->semantic_type;
          } else if(argument.syntax.resolved_type) {
            argument.type = argument.syntax.resolved_type;
          }
          dependent_arguments.push_back(std::move(argument));
        }
        vector<abi_mangle::Type::ClassTemplateArgument> class_arguments;
        TypeMangleContext expression_argument_ctx_storage;
        const TypeMangleContext * expression_argument_ctx = mangle_ctx;
        if(mangle_ctx) {
          expression_argument_ctx_storage = *mangle_ctx;
          expression_argument_ctx_storage
              .prefer_source_template_parameter_expression_arguments = true;
          expression_argument_ctx = &expression_argument_ctx_storage;
        }
        if(!build_dependent_template_arguments_ir(dependent_arguments,
                                                  nullptr,
                                                  expression_argument_ctx,
                                                  class_arguments)) {
          return false;
        }
        member.template_arguments.reserve(class_arguments.size());
        for(size_t i = 0; i < class_arguments.size(); ++i) {
          abi_mangle::TemplateArgument argument;
          if(!class_template_argument_ir_to_template_argument_ir(
                 class_arguments[i],
                 argument)) {
            return false;
          }
          member.template_arguments.push_back(std::move(argument));
        }
      }
      out = member;
      return true;
    }

    if(try_build_current_class_member_expression_ir(node, mangle_ctx, out)) {
      return true;
    }

    if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
      vector<abi_mangle::TemplateArgument> arguments;
      if(!build_dependent_template_id_expression_arguments_ir(*template_id,
                                                              mangle_ctx,
                                                              arguments)) {
        return false;
      }
      out = abi_mangle::DependentExpression::template_id(
          trim_space(template_id->name.name),
          arguments);
      out.suppress_template_prefix_substitution = true;
      return true;
    }

  }
  return false;
}

static bool try_build_non_type_template_parameter_type_ir(
    const TemplateParameterInfo & parameter,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  const auto build_value_type_ir =
      [&]() -> bool
      {
        if(!parameter.value_type) {
          return false;
        }
        abi_mangle::Type value_type_ir;
        if(!try_build_template_argument_type_ir(parameter.value_type,
                                                mangle_ctx,
                                                value_type_ir) ||
           !type_ir_can_be_emitted(value_type_ir)) {
          return false;
        }
        out = std::move(value_type_ir);
        return true;
      };

  const bool dependent_syntax =
      non_type_template_parameter_type_syntax_is_dependent(parameter, mangle_ctx);
  if(parameter.non_type_decl_specifier_seq) {
    CppAstNode source_specifier_seq;
    const CppAstNode * specifier_seq = parameter.non_type_decl_specifier_seq;
    if(dependent_syntax) {
      source_specifier_seq =
          clone_ast_node_for_mangling(*parameter.non_type_decl_specifier_seq);
      clear_default_type_argument_semantics_for_mangling(source_specifier_seq);
      specifier_seq = &source_specifier_seq;
    }
    if(try_build_type_specifier_seq_ast_ir(*specifier_seq,
                                           mangle_ctx,
                                           out)) {
      const CppAstNode * declarator =
          parameter.non_type_declarator ? parameter.non_type_declarator :
                                          parameter.non_type_abstract_declarator;
      if(declarator &&
         (declarator->kind == CppAstKind::declarator ||
          declarator->kind == CppAstKind::abstract_declarator)) {
        for(size_t i = 0; i < declarator->children.size(); ++i) {
          const CppAstNode & child = declarator->children[i];
          if(child.kind != CppAstKind::ptr_operator) {
            continue;
          }
          if(child.has_token && child.simple_type == OP_STAR) {
            out = abi_mangle::Type::pointer(std::move(out));
          } else if(child.has_token && child.simple_type == OP_AMP) {
            out = abi_mangle::Type::lvalue_reference(std::move(out));
          } else if(child.has_token && child.simple_type == OP_LAND) {
            out = abi_mangle::Type::rvalue_reference(std::move(out));
          } else {
            return false;
          }
        }
      }
      if(type_ir_can_be_emitted(out) ||
         (!dependent_syntax && build_value_type_ir())) {
        return true;
      }
      return false;
    } else if(dependent_syntax) {
      return false;
    }
  }
  if(parser_trace::enabled("symbol.linkage")) {
    ostringstream trace;
    trace << "non-type-parameter-type-ir specifier-failed"
          << " parameter=" << parameter.name
          << " decl-text=" << parameter.non_type_decl_specifier_text
          << " has-specifier="
          << (parameter.non_type_decl_specifier_seq ? "yes" : "no")
          << " has-value-type=" << (parameter.value_type ? "yes" : "no")
          << " value-display="
          << (parameter.value_type ? parameter.value_type->named_display : string())
          << " value-key="
          << (parameter.value_type ? parameter.value_type->named_key : string());
    parser_trace::note("symbol.linkage", string(), trace.str());
  }
  if(!parameter.value_type) {
    return false;
  }
  if(build_value_type_ir()) {
    return true;
  }
  if(parser_trace::enabled("symbol.linkage")) {
    ostringstream trace;
    trace << "non-type-parameter-type-ir value-type-failed"
          << " parameter=" << parameter.name
          << " display=" << parameter.value_type->named_display
          << " key=" << parameter.value_type->named_key;
    parser_trace::note("symbol.linkage", string(), trace.str());
  }
  return false;
}

static bool template_value_argument_has_integral_fundamental_type(
    const TemplateArgument & argument)
{
  TypePtr argument_base = strip_top_level_cv(argument.type);
  if(!argument_base || argument_base->kind != Type::TK_FUNDAMENTAL) {
    return false;
  }
  switch(argument_base->fundamental) {
  case FT_FLOAT:
  case FT_DOUBLE:
  case FT_LONG_DOUBLE:
  case FT_VOID:
  case FT_NULLPTR_T:
    return false;
  default:
    break;
  }

  return true;
}

static bool template_value_argument_prefers_concrete_dependent_parameter_value(
    const TemplateArgument & argument,
    const TemplateParameterInfo * parameter,
    const TypeMangleContext * mangle_ctx)
{
  if(!parameter ||
     argument.dependent ||
     !mangle_ctx) {
    return false;
  }
  const bool dependent_parameter_type =
      non_type_template_parameter_type_is_dependent_for_mangling(*parameter,
                                                                 mangle_ctx) ||
      (parameter->value_type &&
       type_has_dependent_mangle_state(parameter->value_type));
  if(!dependent_parameter_type) {
    return false;
  }
  if(mangle_ctx->prefer_concrete_non_type_values_for_dependent_parameter_types) {
    return true;
  }
  if(!template_value_argument_has_integral_fundamental_type(argument)) {
    return false;
  }
  return !non_type_template_parameter_type_mentions_direct_template_parameter(
             *parameter,
             mangle_ctx);
}

static TypePtr template_value_argument_type_for_ir(
    const TemplateArgument & argument,
    const TemplateParameterInfo * parameter,
    const TypeMangleContext * mangle_ctx)
{
  const bool prefer_concrete_dependent_parameter_value =
      template_value_argument_prefers_concrete_dependent_parameter_value(
          argument,
          parameter,
          mangle_ctx);
  if(!prefer_concrete_dependent_parameter_value &&
     parameter &&
     parameter->value_type &&
     type_has_dependent_mangle_state(parameter->value_type)) {
    return parameter->value_type;
  }
  if(argument.type) {
    return argument.type;
  }
  return parameter ? parameter->value_type : TypePtr();
}

static TypePtr class_template_value_argument_type_for_ir(
    const TemplateArgument & argument,
    const TemplateParameterInfo * parameter,
    const TypeMangleContext * mangle_ctx)
{
  TypePtr value_type =
      template_value_argument_type_for_ir(argument, parameter, mangle_ctx);
  if(!argument.dependent &&
     !argument.type &&
     value_type &&
     type_has_dependent_mangle_state(value_type)) {
    return TypePtr();
  }
  return value_type;
}

static bool try_build_dependent_value_template_argument_expression_ir(
    const TemplateArgument & argument,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::DependentExpression & out)
{
  if(argument.source_syntax ||
     (argument.expression &&
      (argument.expression->kind == CppAstKind::id_expression ||
       argument.expression->kind == CppAstKind::identifier))) {
    abi_mangle::Type::ClassTemplateArgument owner_argument;
    const string source_text =
        argument.source_syntax ?
            non_type_template_argument_expression_text_for_ir(
                *argument.source_syntax) :
            argument.expression->value;
    const bool pack_expansion =
        argument.source_syntax && argument.source_syntax->pack_expansion;
    if(try_build_owner_non_type_template_argument_ir(source_text,
                                                     pack_expansion,
                                                     mangle_ctx,
                                                     owner_argument) &&
       owner_argument.kind ==
           abi_mangle::Type::ClassTemplateArgument::
               CTAK_DEPENDENT_EXPRESSION &&
       owner_argument.expression) {
      out = *owner_argument.expression;
      return true;
    }
  }

  if(!argument.dependent) {
    return false;
  }
  bool pack_expansion =
      argument.source_syntax && argument.source_syntax->pack_expansion;
  if(argument.expression &&
     try_build_dependent_expression_ir(*argument.expression, mangle_ctx, out)) {
    if(pack_expansion) {
      out = abi_mangle::DependentExpression::pack_expansion(out);
    }
    return true;
  }
  if(argument.source_syntax &&
     argument.source_syntax->expression &&
     try_build_dependent_expression_ir(*argument.source_syntax->expression,
                                       mangle_ctx,
                                       out)) {
    if(pack_expansion) {
      out = abi_mangle::DependentExpression::pack_expansion(out);
    }
    return true;
  }
  return false;
}

static bool try_build_template_argument_source_type_ir(
    const TemplateArgument & argument,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(!argument.source_syntax) {
    return false;
  }
  if(argument.source_syntax->type_id) {
    if(try_build_type_id_ast_ir(*argument.source_syntax->type_id,
                                mangle_ctx,
                                out)) {
      return true;
    }
  }
  if(argument.source_syntax->resolved_type) {
    if(try_build_template_argument_type_ir(argument.source_syntax->resolved_type,
                                           mangle_ctx,
                                           out)) {
      return true;
    }
  }
  return false;
}

static bool try_find_template_template_parameter_argument_index(
    const TemplateArgument & argument,
    const TypeMangleContext * mangle_ctx,
    size_t & out)
{
  if(argument.kind != TemplateArgument::TA_CLASS_TEMPLATE &&
     argument.kind != TemplateArgument::TA_ALIAS_TEMPLATE) {
    return false;
  }
  if(argument.template_decl && !argument.dependent) {
    return false;
  }

  string text = trim_space(argument.text);
  if(text.empty() && argument.source_syntax) {
    text = trim_space(argument.source_syntax->source_text.empty() ?
                          argument.source_syntax->text :
                          argument.source_syntax->source_text);
  }
  const TemplateParameterInfo * parameter = nullptr;
  return !text.empty() &&
         try_find_template_template_parameter_index(text,
                                                    mangle_ctx,
                                                    out,
                                                    parameter);
}

static bool template_value_argument_integral_value_for_ir(
    const TemplateArgument & argument,
    long long & out)
{
  const string text = trim_space(argument.text);
  if(!text.empty()) {
    if(is_signed_decimal_integer_text(text)) {
      out = parse_signed_decimal_integer_value(text);
      return true;
    }
    if(argument.dependent) {
      return false;
    }
    out = argument.value;
    return true;
  }
  if(argument.dependent) {
    return false;
  }
  out = argument.value;
  return true;
}

static bool try_build_class_template_dependent_integral_value_argument_ir(
    const TemplateArgument & argument,
    const TemplateParameterInfo * parameter,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type::ClassTemplateArgument & out)
{
  if(!parameter ||
     template_value_argument_prefers_concrete_dependent_parameter_value(
         argument,
         parameter,
         mangle_ctx) ||
     !non_type_template_parameter_type_is_dependent_for_mangling(*parameter,
                                                                 mangle_ctx)) {
    return false;
  }

  long long value = 0;
  if(!template_value_argument_integral_value_for_ir(argument, value)) {
    return false;
  }

  abi_mangle::Type parameter_type;
  if(!try_build_non_type_template_parameter_type_ir(*parameter,
                                                    mangle_ctx,
                                                    parameter_type)) {
    return false;
  }
  TypePtr value_type = argument.type;
  if(value_type && !type_has_dependent_mangle_state(value_type)) {
    abi_mangle::Type value_type_ir;
    if(try_build_template_argument_type_ir(value_type, mangle_ctx, value_type_ir)) {
	      out =
	          abi_mangle::Type::ClassTemplateArgument::
	              dependent_integral_value_arg(
	                  std::move(parameter_type),
	                  std::move(value_type_ir),
	                  value);
      return true;
    }
  }
	  out =
	      abi_mangle::Type::ClassTemplateArgument::
	          dependent_untyped_integral_value_arg(std::move(parameter_type),
	                                               value);
  return true;
}

static bool try_build_function_dependent_integral_value_argument_ir(
    const TemplateArgument & argument,
    const TemplateParameterInfo * parameter,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::TemplateArgument & out)
{
  const bool prefer_concrete =
      template_value_argument_prefers_concrete_dependent_parameter_value(
          argument,
          parameter,
          mangle_ctx);
  const bool dependent_parameter =
      parameter &&
      non_type_template_parameter_type_is_dependent_for_mangling(*parameter,
                                                                 mangle_ctx);
  if(!parameter || prefer_concrete || !dependent_parameter) {
    return false;
  }

  long long value = 0;
  if(!template_value_argument_integral_value_for_ir(argument, value)) {
    return false;
  }

  abi_mangle::Type parameter_type;
  if(!try_build_non_type_template_parameter_type_ir(*parameter,
                                                    mangle_ctx,
                                                    parameter_type)) {
    return false;
  }

  TypePtr value_type = argument.type;
  if(value_type && !type_has_dependent_mangle_state(value_type)) {
    abi_mangle::Type value_type_ir;
    if(try_build_template_argument_type_ir(value_type, mangle_ctx, value_type_ir)) {
	      out = abi_mangle::TemplateArgument::dependent_integral_value_arg(
	          std::move(parameter_type),
	          std::move(value_type_ir),
	          value);
      return true;
    }
  }
	  out = abi_mangle::TemplateArgument::dependent_untyped_integral_value_arg(
	      std::move(parameter_type),
	      value);
  return true;
}

static bool try_build_class_template_argument_ir(
    const TemplateArgument & argument,
    const TemplateParameterInfo * parameter,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type::ClassTemplateArgument & out)
{
  switch(argument.kind) {
  case TemplateArgument::TA_TYPE: {
    abi_mangle::Type type;
    if(!try_build_template_argument_type_ir(argument.type, mangle_ctx, type) &&
       !try_build_template_argument_source_type_ir(argument, mangle_ctx, type)) {
      return false;
    }
	    out = abi_mangle::Type::ClassTemplateArgument::type_arg(std::move(type));
    return true;
  }

  case TemplateArgument::TA_VALUE: {
    ExternalEntityArgumentIrPayload entity_payload;
    if(try_build_external_entity_argument_ir_payload(argument,
                                                    mangle_ctx,
                                                    entity_payload)) {
      out = class_external_entity_argument_from_payload(entity_payload);
      return true;
    }
    if(try_build_class_template_dependent_integral_value_argument_ir(argument,
                                                                     parameter,
                                                                     mangle_ctx,
                                                                     out)) {
      return true;
    }
    if(argument.dependent) {
      abi_mangle::Type::ClassTemplateArgument owner_argument;
      const bool pack_expansion =
          argument.source_syntax && argument.source_syntax->pack_expansion;
      const string owner_argument_text =
          argument.source_syntax ?
              non_type_template_argument_expression_text_for_ir(
                  *argument.source_syntax) :
          argument.expression &&
                  (argument.expression->kind == CppAstKind::id_expression ||
                   argument.expression->kind == CppAstKind::identifier) ?
              argument.expression->value :
              argument.text;
      if(try_build_owner_non_type_template_argument_ir(owner_argument_text,
                                                       pack_expansion,
                                                       mangle_ctx,
                                                       owner_argument)) {
        out = std::move(owner_argument);
        return true;
      }
      abi_mangle::DependentExpression expression;
	    if(try_build_dependent_value_template_argument_expression_ir(argument,
	                                                                 mangle_ctx,
	                                                                 expression)) {
	      out = abi_mangle::Type::ClassTemplateArgument::
	          dependent_expression_arg(std::move(expression));
        return true;
      }
      return false;
    }
    TypePtr value_type =
        class_template_value_argument_type_for_ir(argument,
                                                  parameter,
                                                  mangle_ctx);
    if(value_type) {
      abi_mangle::Type type;
      if(!try_build_template_argument_type_ir(value_type, mangle_ctx, type)) {
        return false;
      }
	      out = abi_mangle::Type::ClassTemplateArgument::integral_value_arg(
	          std::move(type),
	          argument.value);
    } else {
      out = abi_mangle::Type::ClassTemplateArgument::untyped_integral_value_arg(
          argument.value);
    }
    return true;
  }

  case TemplateArgument::TA_CLASS_TEMPLATE:
  case TemplateArgument::TA_ALIAS_TEMPLATE: {
    size_t template_parameter_index = 0;
    if(try_find_template_template_parameter_argument_index(
           argument,
           mangle_ctx,
           template_parameter_index)) {
      out = abi_mangle::Type::ClassTemplateArgument::
          template_parameter_template_arg(template_parameter_index);
      return true;
    }

    vector<abi_mangle::Type::NameComponent> prefix_components;
    string template_name;
    string template_name_substitution;
    abi_mangle::Type template_owner;
    if(!try_build_template_entity_argument_name_ir(argument,
                                                   prefix_components,
                                                   template_name,
                                                   template_name_substitution,
                                                   mangle_ctx,
                                                   &template_owner)) {
      return false;
    }
	    out = template_owner.kind == abi_mangle::Type::TK_INVALID ?
	        abi_mangle::Type::ClassTemplateArgument::template_entity_arg(
	            std::move(prefix_components),
	            std::move(template_name),
	            std::move(template_name_substitution)) :
	        abi_mangle::Type::ClassTemplateArgument::member_template_entity_arg(
	            std::move(template_owner),
	            std::move(template_name),
	            std::move(template_name_substitution));
    return true;
  }
  }
  return false;
}

static bool try_build_class_template_arguments_ir(
    const vector<TemplateArgument> & arguments,
    const vector<TemplateParameterInfo> * parameters,
    const map<string, size_t> * pack_sizes,
    const TypeMangleContext * mangle_ctx,
    vector<abi_mangle::Type::ClassTemplateArgument> & out)
{
  out.clear();
  if(!parameters || parameters->empty()) {
    out.reserve(arguments.size());
    for(size_t i = 0; i < arguments.size(); ++i) {
      abi_mangle::Type::ClassTemplateArgument argument;
      if(!try_build_class_template_argument_ir(arguments[i],
                                               nullptr,
                                               mangle_ctx,
                                               argument)) {
        return false;
      }
      out.push_back(std::move(argument));
    }
    return true;
  }

  size_t arg_index = 0;
  for(size_t i = 0; i < parameters->size(); ++i) {
    const TemplateParameterInfo & parameter = (*parameters)[i];
    if(parameter.parameter_pack) {
      size_t trailing_nonpack = 0;
      for(size_t j = i + 1; j < parameters->size(); ++j) {
        if(!(*parameters)[j].parameter_pack) {
          ++trailing_nonpack;
        }
      }
      size_t pack_count = arguments.size() >= arg_index + trailing_nonpack ?
          arguments.size() - arg_index - trailing_nonpack :
          static_cast<size_t>(-1);
      if(pack_sizes) {
        map<string, size_t>::const_iterator found =
            !parameter.name.empty() ? pack_sizes->find(parameter.name) : pack_sizes->end();
        if(found == pack_sizes->end() && !parameter.placeholder_key.empty()) {
          found = pack_sizes->find(parameter.placeholder_key);
        }
        if(found != pack_sizes->end()) {
          pack_count = found->second;
        }
      }
      if(pack_count == static_cast<size_t>(-1) ||
         arguments.size() < arg_index + pack_count + trailing_nonpack) {
        return false;
      }
      vector<abi_mangle::Type::ClassTemplateArgument> pack_arguments;
      pack_arguments.reserve(pack_count);
      for(size_t j = 0; j < pack_count; ++j) {
        abi_mangle::Type::ClassTemplateArgument argument;
        if(!try_build_class_template_argument_ir(arguments[arg_index++],
                                                 &parameter,
                                                 mangle_ctx,
                                                 argument)) {
          return false;
        }
        pack_arguments.push_back(std::move(argument));
      }
      out.push_back(
          abi_mangle::Type::ClassTemplateArgument::argument_pack(
              std::move(pack_arguments)));
      continue;
    }

    if(arg_index >= arguments.size()) {
      return false;
    }
    abi_mangle::Type::ClassTemplateArgument argument;
    if(!try_build_class_template_argument_ir(arguments[arg_index++],
                                             &parameter,
                                             mangle_ctx,
                                             argument)) {
      return false;
    }
    out.push_back(std::move(argument));
  }

  return arg_index == arguments.size();
}

static bool try_build_function_template_argument_ir(
    const TemplateArgument & argument,
    const TemplateParameterInfo * parameter,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::TemplateArgument & out)
{
  switch(argument.kind) {
  case TemplateArgument::TA_TYPE: {
    abi_mangle::Type type;
    if(!try_build_template_argument_type_ir(argument.type, mangle_ctx, type) &&
       !try_build_template_argument_source_type_ir(argument, mangle_ctx, type)) {
      return false;
    }
	    out = abi_mangle::TemplateArgument::type_arg(std::move(type));
    return true;
  }

  case TemplateArgument::TA_VALUE: {
    ExternalEntityArgumentIrPayload entity_payload;
    if(try_build_external_entity_argument_ir_payload(argument,
                                                    mangle_ctx,
                                                    entity_payload)) {
      out = function_external_entity_argument_from_payload(entity_payload);
      return true;
    }
    if(try_build_function_dependent_integral_value_argument_ir(argument,
                                                               parameter,
                                                               mangle_ctx,
                                                               out)) {
      return true;
    }
    if(argument.dependent) {
      abi_mangle::Type::ClassTemplateArgument owner_class_argument;
      abi_mangle::TemplateArgument owner_argument;
      const bool pack_expansion =
          argument.source_syntax && argument.source_syntax->pack_expansion;
      if(try_build_owner_non_type_template_argument_ir(argument.text,
                                                       pack_expansion,
                                                       mangle_ctx,
                                                       owner_class_argument) &&
         class_template_argument_ir_to_template_argument_ir(owner_class_argument,
                                                           owner_argument)) {
        out = std::move(owner_argument);
        return true;
      }
      abi_mangle::DependentExpression expression;
	    if(try_build_dependent_value_template_argument_expression_ir(argument,
	                                                                 mangle_ctx,
	                                                                 expression)) {
	      out = abi_mangle::TemplateArgument::
	          dependent_expression_arg(std::move(expression));
        return true;
      }
      return false;
    }
    TypePtr value_type = template_value_argument_type_for_ir(argument,
                                                            parameter,
                                                            mangle_ctx);
    if(value_type) {
      abi_mangle::Type type;
      if(!try_build_template_argument_type_ir(value_type, mangle_ctx, type)) {
        return false;
      }
	      out = abi_mangle::TemplateArgument::integral_value_arg(
	          std::move(type),
	          argument.value);
    } else {
      out = abi_mangle::TemplateArgument::untyped_integral_value_arg(
          argument.value);
    }
    return true;
  }

  case TemplateArgument::TA_CLASS_TEMPLATE:
  case TemplateArgument::TA_ALIAS_TEMPLATE: {
    size_t template_parameter_index = 0;
    if(try_find_template_template_parameter_argument_index(
           argument,
           mangle_ctx,
           template_parameter_index)) {
      out = abi_mangle::TemplateArgument::
          template_parameter_template_arg(template_parameter_index);
      return true;
    }

    vector<abi_mangle::Type::NameComponent> prefix_components;
    string template_name;
    string template_name_substitution;
    abi_mangle::Type template_owner;
    if(!try_build_template_entity_argument_name_ir(argument,
                                                   prefix_components,
                                                   template_name,
                                                   template_name_substitution,
                                                   mangle_ctx,
                                                   &template_owner)) {
      return false;
    }
	    out = template_owner.kind == abi_mangle::Type::TK_INVALID ?
	        abi_mangle::TemplateArgument::template_entity_arg(
	            std::move(prefix_components),
	            std::move(template_name),
	            std::move(template_name_substitution)) :
	        abi_mangle::TemplateArgument::member_template_entity_arg(
	            std::move(template_owner),
	            std::move(template_name),
	            std::move(template_name_substitution));
    return true;
  }
  }
  return false;
}

static bool try_build_function_template_arguments_ir(
    const vector<TemplateArgument> & arguments,
    const vector<TemplateParameterInfo> * parameters,
    const map<string, size_t> * pack_sizes,
    const TypeMangleContext * mangle_ctx,
    vector<abi_mangle::TemplateArgument> & out)
{
  out.clear();
  if(!parameters || parameters->empty()) {
    out.reserve(arguments.size());
    for(size_t i = 0; i < arguments.size(); ++i) {
      abi_mangle::TemplateArgument argument;
      if(!try_build_function_template_argument_ir(arguments[i],
                                                  nullptr,
                                                  mangle_ctx,
                                                  argument)) {
        return false;
      }
      out.push_back(std::move(argument));
    }
    return true;
  }

  size_t arg_index = 0;
  for(size_t i = 0; i < parameters->size(); ++i) {
    const TemplateParameterInfo & parameter = (*parameters)[i];
    if(parameter.parameter_pack) {
      size_t trailing_nonpack = 0;
      for(size_t j = i + 1; j < parameters->size(); ++j) {
        if(!(*parameters)[j].parameter_pack) {
          ++trailing_nonpack;
        }
      }
      size_t pack_count = arguments.size() >= arg_index + trailing_nonpack ?
          arguments.size() - arg_index - trailing_nonpack :
          static_cast<size_t>(-1);
      if(pack_sizes) {
        map<string, size_t>::const_iterator found =
            !parameter.name.empty() ? pack_sizes->find(parameter.name) : pack_sizes->end();
        if(found == pack_sizes->end() && !parameter.placeholder_key.empty()) {
          found = pack_sizes->find(parameter.placeholder_key);
        }
        if(found != pack_sizes->end()) {
          pack_count = found->second;
        }
      }
      if(pack_count == static_cast<size_t>(-1) ||
         arguments.size() < arg_index + pack_count + trailing_nonpack) {
        return false;
      }
      vector<abi_mangle::TemplateArgument> pack_arguments;
      pack_arguments.reserve(pack_count);
      for(size_t j = 0; j < pack_count; ++j) {
        abi_mangle::TemplateArgument argument;
        if(!try_build_function_template_argument_ir(arguments[arg_index++],
                                                    &parameter,
                                                    mangle_ctx,
                                                    argument)) {
          return false;
        }
        pack_arguments.push_back(std::move(argument));
      }
      out.push_back(abi_mangle::TemplateArgument::argument_pack(
          std::move(pack_arguments)));
      continue;
    }

    if(arg_index >= arguments.size()) {
      if(parameter.default_argument) {
        continue;
      }
      return false;
    }
    abi_mangle::TemplateArgument argument;
    if(!try_build_function_template_argument_ir(arguments[arg_index++],
                                                &parameter,
                                                mangle_ctx,
                                                argument)) {
      if(parser_trace::enabled("symbol.linkage")) {
        ostringstream trace;
        trace << "function-template-argument-ir failed"
              << " index=" << i
              << " parameter=" << parameter.name
              << " kind=" << static_cast<int>(parameter.kind)
              << " pack=" << (parameter.parameter_pack ? "yes" : "no");
        parser_trace::note("symbol.linkage", string(), trace.str());
      }
      return false;
    }
    out.push_back(std::move(argument));
  }

  return arg_index == arguments.size();
}

static bool try_build_class_template_specialization_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(!type ||
     type->kind != Type::TK_NAMED) {
    return false;
  }
  TypePtr base = type;

  shared_ptr<const ClassTemplateSpecializationMangleInfo> specialization =
      named_type_class_template_specialization_mangle_info_const(base);
  const bool has_typed_member_owner =
      base->named_member_owner_type && !base->named_member_name.empty();
  if(!specialization ||
     trim_space(specialization->template_name).empty() ||
     (!has_typed_member_owner &&
      (specialization->template_name_syntax.rooted ||
       specialization->template_name_syntax.name.empty()))) {
    return false;
  }

  vector<abi_mangle::Type::NameComponent> prefix_components;
  string canonical_prefix;
  if(!has_typed_member_owner) {
    const vector<string> & prefix_parts =
        specialization->template_name_syntax.qualifiers;
    for(size_t i = 0; i < prefix_parts.size(); ++i) {
      const string canonical_component = canonical_component_text(prefix_parts[i]);
      if(i == 0 && canonical_component == "std") {
        prefix_components.push_back(
            abi_mangle::Type::NameComponent::std_namespace());
        canonical_prefix = "std";
        continue;
      }
      const string full_name =
          append_qualified_component_text(canonical_prefix, canonical_component);
      prefix_components.push_back(
          abi_mangle::Type::NameComponent::source(prefix_parts[i],
                                                         full_name));
      canonical_prefix = full_name;
    }
  }
  TypeMangleContext specialization_ctx_storage;
  TemplateParameterMangleContext specialization_template_parameter_ctx;
  const TypeMangleContext * specialization_ctx = mangle_ctx;
  if(!specialization->pack_sizes.empty() ||
     !specialization->arguments.empty() ||
     !specialization->mangle_parameters.empty()) {
    specialization_ctx_storage = mangle_ctx ? *mangle_ctx : TypeMangleContext();
    specialization_ctx_storage.template_argument_pack_sizes = nullptr;
    if(!specialization->mangle_parameters.empty()) {
      specialization_template_parameter_ctx.parameters =
          &specialization->mangle_parameters;
      specialization_ctx_storage.template_parameters =
          &specialization_template_parameter_ctx;
    }
    if(!template_arguments_have_dependent_mangle_state(specialization->arguments) &&
       !template_arguments_have_entity_value(specialization->arguments)) {
      specialization_ctx_storage
          .prefer_concrete_non_type_values_for_dependent_parameter_types = true;
    }
    specialization_ctx = &specialization_ctx_storage;
  }

  const vector<TemplateParameterInfo> * parameters =
      specialization->template_parameters.empty() ?
          nullptr :
          &specialization->template_parameters;
  vector<abi_mangle::Type::ClassTemplateArgument> arguments;
  if(!try_build_class_template_arguments_ir(specialization->arguments,
                                            parameters,
                                            specialization_ctx ?
                                                specialization_ctx->template_argument_pack_sizes :
                                                nullptr,
                                            specialization_ctx,
                                            arguments)) {
    return false;
  }

  if(has_typed_member_owner) {
    abi_mangle::Type owner;
    if(!try_build_type_ir(base->named_member_owner_type,
                          specialization_ctx,
                          owner)) {
      return false;
    }
    const string owner_text = selected_named_type_text(base->named_member_owner_type);
	    const string member_template_name_substitution =
	        owner_text.empty() ?
	            string() :
	            append_qualified_component_text(owner_text, base->named_member_name);
	    vector<abi_mangle::SubstitutionKey> argument_keys;
	    if(!(mangle_ctx && mangle_ctx->suppress_type_substitution_keys) &&
	       !build_class_template_argument_ir_substitution_keys(arguments,
	                                                           argument_keys)) {
	      return false;
	    }
	    abi_mangle::Type ir_type =
	        abi_mangle::Type::member_class_template_specialization(
	            std::move(owner),
	            base->named_member_name,
	            member_template_name_substitution,
	            std::move(arguments));
	    abi_mangle::Type::ensure_name_metadata(ir_type)
	        .register_member_expression_template_name = true;
	    if(mangle_ctx && mangle_ctx->suppress_type_substitution_keys) {
	      out = std::move(ir_type);
	      return true;
	    }
	    abi_mangle::set_substitution(
	        ir_type,
        abi_mangle::SubstitutionKey::class_template_specialization(
            0,
            append_qualified_component_text(
                trim_space(specialization->template_scope_prefix),
                canonical_component_text(trim_space(specialization->template_name))),
            std::move(argument_keys)));
    out = std::move(ir_type);
    return true;
  }

  const string template_name = trim_space(specialization->template_name);
  string standard_substitution;
  bool standard_substitution_includes_arguments = false;
  structured_std_standard_substitution_for_template_component(
      template_name,
      specialization->arguments,
      canonical_prefix,
      specialization_ctx,
      standard_substitution,
      standard_substitution_includes_arguments);

	  const string template_name_substitution =
	      standard_substitution.empty() ?
	          append_qualified_component_text(
	              canonical_prefix,
	              canonical_component_text(template_name)) :
	          string();
	  vector<abi_mangle::SubstitutionKey> argument_keys;
	  const bool needs_argument_substitution =
	      !standard_substitution_includes_arguments &&
	      !(mangle_ctx && mangle_ctx->suppress_type_substitution_keys);
	  if(needs_argument_substitution &&
	     !build_class_template_argument_ir_substitution_keys(arguments,
	                                                         argument_keys)) {
	    return false;
	  }
	  const string canonical_template_key =
	      append_qualified_component_text(canonical_prefix,
	                                      canonical_component_text(template_name));
	  abi_mangle::Type ir_type =
	      abi_mangle::Type::class_template_specialization(
	          std::move(prefix_components),
	          std::move(template_name),
	          std::move(template_name_substitution),
	          std::move(arguments),
	          std::move(standard_substitution),
	          standard_substitution_includes_arguments);

	  if(needs_argument_substitution) {
	    abi_mangle::set_substitution(
	        ir_type,
	        abi_mangle::SubstitutionKey::class_template_specialization(
	            0,
	            canonical_template_key,
	            std::move(argument_keys)));
	  }

  out = std::move(ir_type);
  return true;
}

static bool try_build_named_type_ir(const TypePtr & type,
                                    const TypeMangleContext * mangle_ctx,
                                    abi_mangle::Type & out)
{
  if(!type ||
     type->kind != Type::TK_NAMED ||
     type->named_semantic_kind != Type::NSK_ORDINARY ||
     type->named_dependent_qualified_owner ||
     !type->named_dependent_qualified_members.empty() ||
     !type->named_dependent_qualified_member_template_ids.empty() ||
     named_type_lambda_mangle_metadata(type) ||
     should_prefer_unqualified_lexical_named_type(type, mangle_ctx)) {
    return false;
  }

  if(type->named_member_owner_type && !type->named_member_name.empty()) {
    abi_mangle::Type owner;
    if(!try_build_type_ir(type->named_member_owner_type, mangle_ctx, owner)) {
      return false;
    }
    const string selected_text = preferred_named_type_text(type, mangle_ctx);
    if(selected_text.empty()) {
      return false;
    }
    string member_name = type->named_member_name;
	    abi_mangle::Type ir_type =
	        abi_mangle::Type::member_named_type(std::move(owner),
	                                            std::move(member_name),
	                                            string());
    abi_mangle::SubstitutionKey structural_key =
        abi_mangle::SubstitutionKey::named(selected_text);
    abi_mangle::set_substitution(ir_type, std::move(structural_key));
    out = std::move(ir_type);
    return true;
  }

  const string selected_text = preferred_named_type_text(type, mangle_ctx);
  if(selected_text.find('<') != string::npos ||
     selected_text.find('>') != string::npos) {
    return false;
  }
  const QualifiedName & qualified = type->named_qualified_name_syntax;
  if(qualified.rooted || qualified.name.empty()) {
    return false;
  }

  vector<string> parts = qualified.qualifiers;
  parts.push_back(qualified.name);
  if(parts.empty()) {
    return false;
  }

  vector<abi_mangle::Type::NameComponent> prefix_components;
  string canonical_prefix;
  for(size_t i = 0; i + 1 < parts.size(); ++i) {
    const string canonical_component = canonical_component_text(parts[i]);
    if(i == 0 && canonical_component == "std") {
      prefix_components.push_back(
          abi_mangle::Type::NameComponent::std_namespace());
      canonical_prefix = "std";
      continue;
    }
    const string full_name =
        append_qualified_component_text(canonical_prefix, canonical_component);
    prefix_components.push_back(
        abi_mangle::Type::NameComponent::source(parts[i],
                                                       full_name));
    canonical_prefix = full_name;
  }

  const string name = parts.back();
  const string canonical_name =
      append_qualified_component_text(canonical_prefix,
                                      canonical_component_text(name));
	  abi_mangle::Type ir_type =
	      abi_mangle::Type::named_type(std::move(prefix_components),
	                                   name,
	                                   canonical_name);

  abi_mangle::set_substitution(
      ir_type,
      abi_mangle::SubstitutionKey::named(canonical_name));
  out = std::move(ir_type);
  return true;
}

static bool try_build_contextual_local_named_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  (void)mangle_ctx;
  if(!type ||
     type->kind != Type::TK_NAMED ||
     (type->named_key.find("__local_") == string::npos &&
      type->named_display.find("__local_") == string::npos &&
      type->named_key.find("(anonymous namespace)") == string::npos &&
      type->named_display.find("(anonymous namespace)") == string::npos)) {
    return false;
  }

  string selected = type->named_key.find("__local_") != string::npos ||
                    type->named_key.find("(anonymous namespace)") != string::npos ?
      type->named_key :
      type->named_display;
  selected = trim_elaborated_type_prefix(selected);
  if(selected.empty() ||
     selected.find('<') != string::npos) {
    return false;
  }

  const QualifiedName & qualified = type->named_qualified_name_syntax;
  if(!qualified.name.empty() &&
     !qualified.rooted &&
     is_identifier_text_for_mangling(qualified.name)) {
    vector<abi_mangle::Type::NameComponent> prefix_components;
    string canonical_prefix;
    for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
      const string canonical_component =
          canonical_component_text(qualified.qualifiers[i]);
      if(i == 0 && canonical_component == "std") {
        prefix_components.push_back(
            abi_mangle::Type::NameComponent::std_namespace());
        canonical_prefix = "std";
        continue;
      }
      const string full_name =
          append_qualified_component_text(canonical_prefix,
                                          canonical_component);
      prefix_components.push_back(
          abi_mangle::Type::NameComponent::source(
              qualified.qualifiers[i],
              full_name));
      canonical_prefix = full_name;
    }
    const string canonical_name =
        append_qualified_component_text(canonical_prefix,
                                        canonical_component_text(qualified.name));
	    out = abi_mangle::Type::named_type(std::move(prefix_components),
	                                       qualified.name,
	                                       canonical_name);
    abi_mangle::set_substitution(
        out,
        abi_mangle::SubstitutionKey::named(canonical_name));
    return true;
  }

  if(selected.find("::") != string::npos ||
     !is_identifier_text_for_mangling(selected)) {
    return false;
  }

  const string canonical_name = canonical_component_text(selected);
	  out = abi_mangle::Type::named_type(
	      vector<abi_mangle::Type::NameComponent>(),
	      std::move(selected),
	      canonical_name);
  abi_mangle::set_substitution(
      out,
      abi_mangle::SubstitutionKey::named(canonical_name));
  return true;
}

static bool try_build_wrapped_type_ir(const TypePtr & type,
                                      const TypeMangleContext * mangle_ctx,
                                      abi_mangle::Type & out)
{
  if(!type) {
    return false;
  }
  switch(type->kind) {
  case Type::TK_CV: {
    abi_mangle::Type inner;
    if(!try_build_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::cv(type->cv_const,
                               type->cv_volatile,
                               std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_POINTER: {
    abi_mangle::Type inner;
    if(!try_build_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::pointer(std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_LVALUE_REFERENCE: {
    abi_mangle::Type inner;
    if(!try_build_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::lvalue_reference(std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_RVALUE_REFERENCE: {
    abi_mangle::Type inner;
    if(!try_build_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::rvalue_reference(std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_ARRAY: {
    string bound;
    if(!build_array_bound_type_mangle(type, bound)) {
      return false;
    }
    string bound_key;
    if(!build_array_bound_substitution_key(type, mangle_ctx, bound_key)) {
      return false;
    }
    abi_mangle::Type inner;
    if(!try_build_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::array(std::move(bound),
                                  std::move(bound_key),
                                  std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_FUNCTION: {
    abi_mangle::Type result;
    if(!try_build_type_ir(type->inner, mangle_ctx, result)) {
      return false;
    }
    vector<abi_mangle::Type> params;
    params.reserve(type->params.size());
    for(size_t i = 0; i < type->params.size(); ++i) {
      abi_mangle::Type param;
      if(!try_build_type_ir(type->params[i], mangle_ctx, param)) {
        return false;
      }
      params.push_back(std::move(param));
    }
    out = abi_mangle::Type::function(
        std::move(result),
        std::move(params),
        type->variadic,
        type->function_ref_qualifier == FTRQ_LVALUE,
        type->function_ref_qualifier == FTRQ_RVALUE);
    apply_function_type_cv_ir(type, out);
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_MEMBER_POINTER: {
    abi_mangle::Type owner;
    abi_mangle::Type member;
    if(!try_build_type_ir(type->owner, mangle_ctx, owner) ||
       !try_build_type_ir(type->inner, mangle_ctx, member)) {
      return false;
    }
    out = abi_mangle::Type::member_pointer(std::move(owner),
                                           std::move(member));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_ATOMIC: {
    abi_mangle::Type inner;
    if(!try_build_type_ir(type->inner, mangle_ctx, inner)) {
      return false;
    }
    out = abi_mangle::Type::vendor_qualified("_Atomic", std::move(inner));
    return attach_semantic_type_ir_substitution(type, mangle_ctx, out);
  }

  case Type::TK_FUNDAMENTAL:
  case Type::TK_NAMED:
  case Type::TK_BLOCK_POINTER:
    return false;
  }
  return false;
}

static bool try_build_type_ir(const TypePtr & type,
                              const TypeMangleContext * mangle_ctx,
                              abi_mangle::Type & out)
{
  if(try_build_context_free_type_ir(type, mangle_ctx, out)) {
    return true;
  }
  if(!type) {
    return false;
  }
  if(type->kind != Type::TK_NAMED) {
    if(type->kind == Type::TK_CV &&
       try_build_dependent_decltype_type_ir(type, mangle_ctx, out)) {
      return true;
    }
    return try_build_wrapped_type_ir(type, mangle_ctx, out);
  }
  if(try_build_template_parameter_type_ir(type, mangle_ctx, out)) {
    return true;
  }
  if(try_build_unbound_template_parameter_type_ir(type, out)) {
    return true;
  }
  if(try_build_dependent_decltype_type_ir(type, mangle_ctx, out)) {
    return true;
  }
  if(try_build_dependent_builtin_type_transform_type_ir(type,
                                                        mangle_ctx,
                                                        out)) {
    return true;
  }
  if(try_build_itanium_abi_lambda_closure_type_ir(type, mangle_ctx, out)) {
    return true;
  }
  if(try_build_dependent_alias_type_ir(type, mangle_ctx, out)) {
    return true;
  }
  shared_ptr<const ClassTemplateSpecializationMangleInfo> specialization =
      named_type_class_template_specialization_mangle_info_const(type);
  if(specialization &&
     !template_arguments_have_dependent_mangle_state(specialization->arguments) &&
     !template_arguments_have_entity_value(specialization->arguments) &&
     try_build_class_template_specialization_type_ir(type, mangle_ctx, out)) {
    return true;
  }
  if(try_build_dependent_template_template_parameter_type_ir(type,
                                                            mangle_ctx,
                                                            out)) {
    return true;
  }
  if(try_build_dependent_class_template_type_ir(type, mangle_ctx, out)) {
    return true;
  }
  if(try_build_dependent_qualified_member_type_ir(type, mangle_ctx, out)) {
    return true;
  }
  if(try_build_class_template_specialization_type_ir(type, mangle_ctx, out)) {
    return true;
  }
  if(try_build_contextual_local_named_type_ir(type, mangle_ctx, out)) {
    return true;
  }
  if(try_build_named_type_ir(type, mangle_ctx, out)) {
    return true;
  }
  return try_build_wrapped_type_ir(type, mangle_ctx, out);
}

static bool try_build_type_ir_cached(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::Type & out)
{
  if(!type) {
    return try_build_type_ir(type, mangle_ctx, out);
  }

  if(mangle_ctx && !context_free_type_ir_only_context(mangle_ctx)) {
    return try_build_type_ir(type, mangle_ctx, out);
  }

  static thread_local unordered_map<TypePtr, abi_mangle::Type> default_cache;
  static thread_local unordered_map<TypePtr, abi_mangle::Type> ir_only_cache;
  unordered_map<TypePtr, abi_mangle::Type> & cache =
      mangle_ctx ? ir_only_cache : default_cache;
  unordered_map<TypePtr, abi_mangle::Type>::const_iterator found =
      cache.find(type);
  if(found != cache.end()) {
    out = found->second;
    return true;
  }

  abi_mangle::Type candidate;
  if(!try_build_type_ir(type, mangle_ctx, candidate)) {
    return false;
  }
  cache[type] = candidate;
  out = candidate;
  return true;
}

static bool try_mangle_context_free_type_ir(const TypePtr & type,
                                            string & out,
                                            const TypeMangleContext * mangle_ctx,
                                            MangleSubstitutionState * state)
{
  abi_mangle::Type ir_type;
  if(!try_build_type_ir_cached(type, mangle_ctx, ir_type)) {
    return false;
  }
  MangleIrSubstitutionSink sink(state);
  return abi_mangle::emit_type(ir_type, out, &sink);
}

static bool type_ir_substitution_key_for_type(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::SubstitutionKey & key)
{
  abi_mangle::Type ir_type;
  return try_build_type_ir(type, mangle_ctx, ir_type) &&
         abi_mangle::make_type_substitution_key(ir_type, key) &&
         !key.empty();
}

static bool type_is_direct_template_parameter_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx)
{
  abi_mangle::Type ir_type;
  return try_build_template_parameter_type_ir(type, mangle_ctx, ir_type) &&
         ir_type.kind == abi_mangle::Type::TK_TEMPLATE_PARAMETER;
}

static bool build_type_substitution_key_impl(const TypePtr & type,
                                             const TypeMangleContext * mangle_ctx,
                                             string & out,
                                             bool allow_fundamental_atom);
static string preferred_named_type_text(const TypePtr & type,
                                        const TypeMangleContext * mangle_ctx);
static bool should_prefer_unqualified_lexical_named_type(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx);

static bool build_array_bound_text_key(const string & bound_text,
                                       string & out)
{
  out.clear();
  const string bound = trim_space(bound_text);
  if(bound.empty()) {
    out = "expr:unknown";
    return true;
  }
  if(!is_signed_decimal_integer_text(bound) ||
     (!bound.empty() && bound[0] == '-')) {
    return false;
  }
  out = string("expr:int(") + (bound[0] == '+' ? bound.substr(1) : bound) + ")";
  return true;
}

static bool build_array_bound_substitution_key(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    string & out)
{
  out.clear();
  if(!type || type->kind != Type::TK_ARRAY) {
    return false;
  }
  if(type->has_bound) {
    out = string("expr:int(") + to_string(type->bound) + ")";
    return true;
  }
  if(type->bound_text.empty()) {
    out = "expr:unknown";
    return true;
  }
  return build_array_bound_text_key(type->bound_text, out) ||
         build_template_parameter_index_expression_key(type->bound_text,
                                                       mangle_ctx,
                                                       out);
}

static bool build_array_bound_type_mangle(const TypePtr & type,
                                          string & out)
{
  out.clear();
  if(!type || type->kind != Type::TK_ARRAY) {
    return false;
  }
  if(type->has_bound) {
    out = to_string(type->bound);
  } else {
    out = trim_space(type->bound_text);
  }
  if(out.empty()) {
    return true;
  }
  if(!is_signed_decimal_integer_text(out) || out[0] == '-') {
    return false;
  }
  if(out[0] == '+') {
    out.erase(0, 1);
  }
  return true;
}

static bool try_mangle_value_template_argument_impl(const TypePtr & type,
                                                    long long value,
                                                    string & out,
                                                    MangleSubstitutionState * state)
{
  TypePtr base = strip_top_level_cv(type);
  if(base) {
    abi_mangle::Type ir_type;
    if(try_build_type_ir_cached(base, nullptr, ir_type)) {
      MangleIrSubstitutionSink sink(state);
      return abi_mangle::emit_template_argument(
          abi_mangle::TemplateArgument::integral_value_arg(ir_type, value),
          out,
          &sink);
    }
  }
  out += 'L';
  if(base && try_emit_type_encoding_ir_impl(base, out, nullptr, state)) {
    out += to_string(value);
    out += 'E';
    return true;
  }
  out.pop_back();
  out += "Li";
  out += to_string(value);
  out += 'E';
  return true;
}

static bool try_mangle_untyped_integral_template_argument_value(long long value,
                                                                string & out)
{
  return abi_mangle::emit_template_argument(
      abi_mangle::TemplateArgument::untyped_integral_value_arg(value),
      out,
      nullptr);
}

static string non_type_template_parameter_decl_specifier_text(
    const TemplateParameterInfo & parameter)
{
  return trim_space(parameter.non_type_decl_specifier_text);
}

static string non_type_template_parameter_type_diagnostic_text(
    const TemplateParameterInfo & parameter)
{
  string out = non_type_template_parameter_decl_specifier_text(parameter);
  if(out.empty() && parameter.value_type) {
    out = describe_type(parameter.value_type);
  }
  return out;
}

static bool non_type_template_parameter_type_is_dependent_for_mangling(
    const TemplateParameterInfo & parameter,
    const TypeMangleContext * mangle_ctx)
{
  if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE) {
    return false;
  }
  if(non_type_template_parameter_type_syntax_is_dependent(parameter, mangle_ctx)) {
    return true;
  }
  if(parameter.value_type && type_has_dependent_mangle_state(parameter.value_type)) {
    return true;
  }
  const string text = non_type_template_parameter_decl_specifier_text(parameter);
  return text_mentions_template_mangle_parameter(text, mangle_ctx);
}

static bool try_mangle_non_type_template_parameter_type(
    const TemplateParameterInfo & parameter,
    string & out,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state)
{
  abi_mangle::Type parameter_type;
  if(!try_build_non_type_template_parameter_type_ir(parameter,
                                                    mangle_ctx,
                                                    parameter_type)) {
    return false;
  }
  MangleIrSubstitutionSink sink(state);
  return abi_mangle::emit_type(parameter_type, out, &sink);
}

static bool try_mangle_dependent_non_type_template_argument(
    const TemplateArgument & arg,
    const TemplateParameterInfo & parameter,
    string & out,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state,
    size_t parameter_index)
{
  const bool dependent_parameter_type =
      non_type_template_parameter_type_is_dependent_for_mangling(parameter, mangle_ctx);
  if(!dependent_parameter_type) {
    return false;
  }

  struct MergeTemplateParameterSubstitutions
  {
    static void merge(const MangleSubstitutionState & source,
                      MangleSubstitutionState * target)
    {
      if(!target) {
        return;
      }
      const size_t first_new_index = target->substitution_keys.size();
      size_t first_template_parameter_index = source.substitution_keys.size();
      bool has_template_prefix_before_parameter = false;
      bool has_decay_alias_after_parameter = false;
      for(size_t i = first_new_index; i < source.substitution_keys.size(); ++i) {
        const string & key = source.substitution_keys[i];
        const abi_mangle::SubstitutionKey & ir_key =
            substitution_ir_key_at(source, i);
        if(substitution_slot_is_template_parameter(key, ir_key)) {
          if(first_template_parameter_index == source.substitution_keys.size()) {
            first_template_parameter_index = i;
          }
          continue;
        }
        if(i < first_template_parameter_index &&
           substitution_slot_is_template_prefix(key, ir_key)) {
          has_template_prefix_before_parameter = true;
        } else if(i > first_template_parameter_index &&
                  substitution_slot_is_decay_alias_name(key, ir_key)) {
          has_decay_alias_after_parameter = true;
        }
      }

      bool merged_primary_prefix = false;
      const bool has_template_parameter_key =
          first_template_parameter_index != source.substitution_keys.size();
      for(size_t i = first_new_index; i < source.substitution_keys.size(); ++i) {
        const string & key = source.substitution_keys[i];
        const abi_mangle::SubstitutionKey & ir_key =
            substitution_ir_key_at(source, i);
        if(i < first_template_parameter_index) {
          if(!has_template_parameter_key) {
            register_substitution_slot_if_absent(target, key, ir_key);
          } else if(has_decay_alias_after_parameter) {
            if(substitution_slot_is_template_prefix(key, ir_key)) {
              register_substitution_slot_if_absent(target, key, ir_key);
              merged_primary_prefix = true;
            } else if(!has_template_prefix_before_parameter &&
                      !merged_primary_prefix &&
                      substitution_slot_is_name(key, ir_key)) {
              register_substitution_slot_if_absent(target, key, ir_key);
              merged_primary_prefix = true;
            }
          } else if(substitution_slot_is_template_prefix(key, ir_key) ||
                    substitution_slot_is_name(key, ir_key)) {
            register_substitution_slot_if_absent(target, key, ir_key);
          }
          continue;
        }
        register_substitution_slot_if_absent(target, key, ir_key);
      }
    }
  };

  const size_t begin = out.size();
  out += "Tn";
  MangleSubstitutionState parameter_type_state;
  MangleSubstitutionState * parameter_type_state_ptr = state;
  if(state) {
    parameter_type_state = *state;
    parameter_type_state_ptr = &parameter_type_state;
  }
  const size_t type_begin = out.size();
  if(!try_mangle_non_type_template_parameter_type(
         parameter, out, mangle_ctx, parameter_type_state_ptr) ||
     out.size() == type_begin) {
    out.resize(begin);
    if(!arg.dependent &&
       arg.type &&
       !type_has_dependent_mangle_state(arg.type)) {
      return false;
    }
    throw_unstructured_dependent_text_mangling(
        "non-type template parameter type",
        non_type_template_parameter_type_diagnostic_text(parameter));
  }
  MergeTemplateParameterSubstitutions::merge(parameter_type_state, state);
  if(!arg.dependent) {
    if(arg.type &&
       try_mangle_value_template_argument_impl(arg.type, arg.value, out, state)) {
      return true;
    }
    return try_mangle_untyped_integral_template_argument_value(arg.value, out);
  }
  if(arg.expression) {
    return try_mangle_dependent_expression_ast_template_argument(
        *arg.expression, out, mangle_ctx, state);
  }
  if(!arg.text.empty()) {
    if(try_mangle_non_type_template_parameter_expression_pack_text(arg.text,
                                                                    out,
                                                                    mangle_ctx,
                                                                    state)) {
      return true;
    }
    if(try_mangle_non_type_template_parameter_expression_name(arg.text,
                                                              out,
                                                              mangle_ctx,
                                                              state)) {
      return true;
    }
    // Dependent class-template metadata is positional; out-of-class definitions
    // may use different parameter names than the primary template.
    if(parameter_index != static_cast<size_t>(-1) &&
       is_identifier_text_for_mangling(arg.text)) {
      return emit_template_parameter_index(parameter_index,
                                           parameter,
                                           out,
                                           state,
                                           false);
    }
    throw_unstructured_dependent_text_mangling(
        "non-type template argument",
        arg.text);
  }
  return try_mangle_value_template_argument_impl(TypePtr(), arg.value, out, state);
}

static bool try_mangle_template_parameter_type(const TypePtr & type,
                                               const TypeMangleContext * mangle_ctx,
                                               string & out,
                                               MangleSubstitutionState * state = nullptr)
{
  if(!mangle_ctx || !type || type->kind != Type::TK_NAMED ||
     ((!mangle_ctx->template_parameters ||
       !mangle_ctx->template_parameters->parameters) &&
      !mangle_ctx->owner_template_parameters)) {
    return false;
  }

  const string named_display = trim_elaborated_type_prefix(type->named_display);
  const bool has_template_parameter_key =
      type->named_semantic_kind == Type::NSK_TEMPLATE_PARAMETER;
  if(mangle_ctx->template_parameters &&
     mangle_ctx->template_parameters->parameters) {
    for(size_t i = 0; i < mangle_ctx->template_parameters->parameters->size(); ++i) {
      const TemplateParameterInfo & param = (*mangle_ctx->template_parameters->parameters)[i];
      if(param.kind != TemplateParameterInfo::TP_TYPE) {
        continue;
      }
      if((!param.placeholder_key.empty() && type->named_key == param.placeholder_key) ||
         (has_template_parameter_key &&
          !param.name.empty() &&
          text_matches_type_parameter_name(named_display, param.name))) {
        if(state &&
           !param.name.empty() &&
           (!mangle_ctx ||
            !mangle_ctx->suppress_template_parameter_type_registration) &&
           emit_registered_substitution(
               state,
               template_parameter_type_substitution_key(
                   mangle_ctx->template_parameters->parameters, i, param),
               out)) {
          return true;
        }
        out += 'T';
        if(i > 0) {
          out += to_string(i - 1);
        }
        out += '_';
        return true;
      }
    }
  }
  if(mangle_ctx->owner_template_parameters &&
     mangle_ctx->owner_template_arguments) {
    const vector<TemplateParameterInfo> & parameters =
        *mangle_ctx->owner_template_parameters;
    const vector<TemplateArgument> & arguments =
        *mangle_ctx->owner_template_arguments;
    for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
      const TemplateParameterInfo & param = parameters[i];
      if(param.kind != TemplateParameterInfo::TP_TYPE || param.name.empty()) {
        continue;
      }
      if((!param.placeholder_key.empty() && type->named_key == param.placeholder_key) ||
         (has_template_parameter_key &&
          text_matches_type_parameter_name(named_display, param.name))) {
        if(owner_template_argument_index_is_suppressed(mangle_ctx, i)) {
          return emit_template_parameter_index(
              i,
              param,
              out,
              state,
              !mangle_ctx->suppress_template_parameter_type_registration,
              &parameters);
        }
        if(template_argument_is_self_type_parameter(
               arguments[i], param, type, named_display)) {
          return emit_template_parameter_index(
              i,
              param,
              out,
              state,
              !mangle_ctx->suppress_template_parameter_type_registration,
              &parameters);
        }
        TypeMangleContext owner_arg_ctx_storage;
        return try_mangle_template_argument_impl(
            arguments[i],
            &param,
            out,
            suppress_owner_template_argument_index(
                mangle_ctx, i, owner_arg_ctx_storage),
            state);
      }
    }
  }
  return false;
}

static bool should_prefer_unqualified_lexical_named_type(const TypePtr & type,
                                                         const TypeMangleContext * mangle_ctx)
{
  if(!type || type->kind != Type::TK_NAMED || !mangle_ctx ||
     mangle_ctx->lexical_scope.empty() ||
     !lexical_scope_supports_local_component_emission(mangle_ctx->lexical_scope)) {
    return false;
  }

  const string display_text = trim_elaborated_type_prefix(type->named_display);
  if(type->named_source_name.empty() ||
     display_text != type->named_source_name) {
    return false;
  }

  const string key_text = trim_elaborated_type_prefix(type->named_key);
  if(key_text.empty()) {
    return false;
  }

  string canonical_scoped_text;
  string canonical_key_text;
  const string scoped_display_text =
      append_qualified_component_text(mangle_ctx->lexical_scope, display_text);
  return canonicalize_named_substitution_text(scoped_display_text,
                                              canonical_scoped_text) &&
         canonicalize_named_substitution_text(key_text, canonical_key_text) &&
         canonical_scoped_text == canonical_key_text;
}

static bool try_emit_type_encoding_ir(const TypePtr & type,
                            string & out,
                            const TypeMangleContext * mangle_ctx)
{
  MangleSubstitutionState state;
  return try_emit_type_encoding_ir_impl(type, out, mangle_ctx, &state);
}

static bool parse_type_substitution_wrapper_key(const string & key,
                                                string & inner)
{
  if(key.compare(0, 8, "type:cv(") == 0 &&
     !key.empty() &&
     key[key.size() - 1] == ')') {
    const size_t comma = key.find(',', 8);
    if(comma == string::npos || comma + 1 >= key.size() - 1) {
      return false;
    }
    inner = key.substr(comma + 1, key.size() - comma - 2);
    return true;
  }

  static const char * const kPrefixes[] = {
      "type:ptr(",
      "type:lref(",
      "type:rref(",
  };
  for(size_t i = 0; i < sizeof(kPrefixes) / sizeof(kPrefixes[0]); ++i) {
    const string prefix = kPrefixes[i];
    if(key.compare(0, prefix.size(), prefix) != 0 ||
       key.empty() ||
       key[key.size() - 1] != ')') {
      continue;
    }
    inner = key.substr(prefix.size(), key.size() - prefix.size() - 1);
    return true;
  }

  const string array_prefix = "type:array(";
  if(key.compare(0, array_prefix.size(), array_prefix) == 0 &&
     !key.empty() &&
     key[key.size() - 1] == ')') {
    const size_t payload_begin = array_prefix.size();
    const size_t payload_end = key.size() - 1;
    int angle_depth = 0;
    int paren_depth = 0;
    for(size_t i = payload_begin; i < payload_end; ++i) {
      const char ch = key[i];
      if(ch == '<') {
        ++angle_depth;
      } else if(ch == '>') {
        if(angle_depth > 0) {
          --angle_depth;
        }
      } else if(ch == '(') {
        ++paren_depth;
      } else if(ch == ')') {
        if(paren_depth > 0) {
          --paren_depth;
        }
      } else if(ch == ',' && angle_depth == 0 && paren_depth == 0) {
        if(i + 1 >= payload_end) {
          return false;
        }
        inner = key.substr(i + 1, payload_end - i - 1);
        return true;
      }
    }
  }

  return false;
}

static void register_function_type_prerequisite_keys(
    const string & substitution_key,
    MangleSubstitutionState * state)
{
  string inner;
  if(!parse_type_substitution_wrapper_key(substitution_key, inner)) {
    return;
  }
  register_function_type_prerequisite_keys(inner, state);
  if(inner.find("type:fn(") == 0) {
    register_substitution_key_if_absent(state, inner);
  }
}

static bool try_emit_type_encoding_ir_impl(const TypePtr & type,
                                           string & out,
                                           const TypeMangleContext * mangle_ctx,
                                           MangleSubstitutionState * state)
{
  if(!type) {
    return false;
  }
  TypeMangleRecursionGuard recursion_guard(type.get());
  if(!recursion_guard.entered()) {
    return false;
  }
  return try_mangle_context_free_type_ir(type, out, mangle_ctx, state);
}

static string selected_named_type_text(const TypePtr & type)
{
  if(!type || type->kind != Type::TK_NAMED) {
    return string();
  }

  const string display_text = trim_elaborated_type_prefix(type->named_display);
  const string key_text = trim_elaborated_type_prefix(type->named_key);
  return named_type_should_prefer_key_for_mangling(*type, display_text, key_text) ?
      key_text :
      display_text;
}

static void adopt_actual_class_template_owner_for_hybrid_named_type(
    const TypePtr & hybrid,
    const TypePtr & actual_type)
{
  void * pattern_decl = nullptr;
  vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
  const bool has_dependent_template =
      named_type_dependent_class_template(hybrid,
                                          pattern_decl,
                                          dependent_arguments);
  shared_ptr<const ClassTemplateSpecializationMangleInfo> pattern_info =
      named_type_class_template_specialization_mangle_info_const(hybrid);
  if(!has_dependent_template && !pattern_info) {
    return;
  }

  shared_ptr<const ClassTemplateSpecializationMangleInfo> actual_info =
      named_type_class_template_specialization_mangle_info_const(actual_type);
  if(!actual_info) {
    return;
  }
  if(pattern_info &&
     !trim_space(pattern_info->template_name).empty() &&
     !trim_space(actual_info->template_name).empty() &&
     trim_space(pattern_info->template_name) !=
         trim_space(actual_info->template_name)) {
    return;
  }

  shared_ptr<ClassTemplateSpecializationMangleInfo> hybrid_info(
      new ClassTemplateSpecializationMangleInfo(*actual_info));
  if(pattern_info) {
    if(!pattern_info->arguments.empty()) {
      hybrid_info->arguments = pattern_info->arguments;
    }
    if(!pattern_info->template_parameters.empty()) {
      hybrid_info->template_parameters = pattern_info->template_parameters;
    }
    if(!pattern_info->mangle_parameters.empty()) {
      hybrid_info->mangle_parameters = pattern_info->mangle_parameters;
    }
    if(!pattern_info->argument_syntaxes.empty()) {
      hybrid_info->argument_syntaxes = pattern_info->argument_syntaxes;
    }
    if(!pattern_info->pack_sizes.empty()) {
      hybrid_info->pack_sizes = pattern_info->pack_sizes;
    }
  }
  set_named_type_class_template_specialization_mangle_info(hybrid, hybrid_info);

  if(!has_dependent_template || !actual_info->class_template_decl) {
    return;
  }

  const semantic_model::ClassTemplateDecl * pattern_template =
      static_cast<const semantic_model::ClassTemplateDecl *>(pattern_decl);
  const semantic_model::ClassTemplateDecl * actual_template =
      static_cast<const semantic_model::ClassTemplateDecl *>(
          actual_info->class_template_decl);
  if(pattern_template &&
     actual_template &&
     pattern_template->name != actual_template->name) {
    return;
  }

  set_named_type_dependent_class_template(hybrid,
                                          actual_info->class_template_decl,
                                          dependent_arguments);
}

static bool named_type_is_concrete_owner_type_parameter(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx)
{
  if(!type || type->kind != Type::TK_NAMED ||
     !mangle_ctx ||
     !mangle_ctx->owner_template_parameters ||
     !mangle_ctx->owner_template_arguments) {
    return false;
  }
  const string selected = selected_named_type_text(type);
  if(selected.empty()) {
    return false;
  }
  const vector<TemplateParameterInfo> & parameters =
      *mangle_ctx->owner_template_parameters;
  const vector<TemplateArgument> & arguments =
      *mangle_ctx->owner_template_arguments;
  for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
    const TemplateParameterInfo & parameter = parameters[i];
    if(parameter.kind != TemplateParameterInfo::TP_TYPE ||
       parameter.name.empty() ||
       !text_matches_type_parameter_name(selected, parameter.name)) {
      continue;
    }
    return !template_argument_is_self_type_parameter(arguments[i],
                                                    parameter,
                                                    TypePtr(),
                                                    selected);
  }
  return false;
}

static TypePtr hybridize_pattern_type_with_actual(const TypePtr & pattern_type,
                                                  const TypePtr & actual_type,
                                                  const TypeMangleContext * mangle_ctx)
{
  if(!pattern_type) {
    return TypePtr();
  }

  switch(pattern_type->kind) {
  case Type::TK_NAMED: {
    if(actual_type &&
       !type_has_dependent_mangle_state(actual_type) &&
       named_type_is_concrete_owner_type_parameter(pattern_type, mangle_ctx)) {
      return actual_type;
    }

    string template_param_code;
    if(try_mangle_template_parameter_type(pattern_type, mangle_ctx, template_param_code)) {
      return pattern_type;
    }
    if(actual_type &&
       !type_has_dependent_mangle_state(actual_type) &&
       pattern_type->named_semantic_kind == Type::NSK_TEMPLATE_PARAMETER) {
      return actual_type;
    }

    if(actual_type &&
       !type_has_dependent_mangle_state(actual_type) &&
       !type_has_dependent_mangle_state(pattern_type)) {
      return actual_type;
    }

    if(actual_type) {
      TypePtr hybrid(new Type(*pattern_type));
      adopt_actual_class_template_owner_for_hybrid_named_type(hybrid,
                                                             actual_type);
      if(named_type_class_template_specialization_mangle_info_const(hybrid)) {
        return hybrid;
      }
    }
    return pattern_type;
  }

  case Type::TK_CV:
  case Type::TK_POINTER:
  case Type::TK_MEMBER_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
  case Type::TK_ATOMIC:
  case Type::TK_FUNCTION: {
    TypePtr hybrid(new Type(*pattern_type));
    TypePtr actual_inner = actual_type;
    if(actual_inner && actual_inner->kind == pattern_type->kind) {
      actual_inner = actual_inner->inner;
    } else if(actual_inner && pattern_type->kind == Type::TK_CV &&
              actual_inner->kind == Type::TK_CV) {
      actual_inner = actual_inner->inner;
    } else if(actual_inner && pattern_type->kind != Type::TK_FUNCTION &&
              pattern_type->kind != Type::TK_ARRAY &&
              pattern_type->kind != Type::TK_MEMBER_POINTER &&
              pattern_type->kind != Type::TK_BLOCK_POINTER &&
              pattern_type->kind != Type::TK_ATOMIC) {
      actual_inner = TypePtr();
    }

    hybrid->inner = hybridize_pattern_type_with_actual(pattern_type->inner,
                                                       actual_inner,
                                                       mangle_ctx);
    for(size_t i = 0; i < pattern_type->params.size(); ++i) {
      TypePtr actual_param =
          actual_type && actual_type->kind == Type::TK_FUNCTION && i < actual_type->params.size() ?
              actual_type->params[i] :
              TypePtr();
      hybrid->params[i] = hybridize_pattern_type_with_actual(pattern_type->params[i],
                                                             actual_param,
                                                             mangle_ctx);
    }
    return hybrid;
  }

  default:
    return pattern_type;
  }
}

static bool type_has_dependent_mangle_state(const TypePtr & type)
{
  if(!type) {
    return false;
  }

  if(type->kind == Type::TK_NAMED &&
     (named_type_has_dependent_semantic(type) ||
      named_type_key_contains_dependent_semantic(type))) {
    return true;
  }

  if(type_has_dependent_mangle_state(type->owner) ||
     type_has_dependent_mangle_state(type->inner)) {
    return true;
  }

  for(size_t i = 0; i < type->params.size(); ++i) {
    if(type_has_dependent_mangle_state(type->params[i])) {
      return true;
    }
  }

  return false;
}

static bool template_argument_has_dependent_mangle_state(
    const TemplateArgument & argument)
{
  if(argument.dependent) {
    return true;
  }

  switch(argument.kind) {
  case TemplateArgument::TA_TYPE:
    return type_has_dependent_mangle_state(argument.type);

  case TemplateArgument::TA_VALUE:
    return type_has_dependent_mangle_state(argument.type);

  case TemplateArgument::TA_CLASS_TEMPLATE:
  case TemplateArgument::TA_ALIAS_TEMPLATE:
    return argument.template_decl == nullptr ||
           type_has_dependent_mangle_state(argument.template_owner_type);
  }

  return false;
}

static bool template_arguments_have_dependent_mangle_state(
    const vector<TemplateArgument> & arguments)
{
  for(size_t i = 0; i < arguments.size(); ++i) {
    if(template_argument_has_dependent_mangle_state(arguments[i])) {
      return true;
    }
  }
  return false;
}

static bool template_arguments_have_entity_value(
    const vector<TemplateArgument> & arguments)
{
  for(size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].kind == TemplateArgument::TA_VALUE &&
       (arguments[i].function_value || arguments[i].value_binding)) {
      return true;
    }
  }
  return false;
}

static bool type_contains_lambda_mangle_metadata(const TypePtr & type)
{
  if(!type) {
    return false;
  }

  if(type->kind == Type::TK_NAMED && named_type_lambda_mangle_metadata(type)) {
    return true;
  }

  if(type_contains_lambda_mangle_metadata(type->owner) ||
     type_contains_lambda_mangle_metadata(type->inner)) {
    return true;
  }

  for(size_t i = 0; i < type->params.size(); ++i) {
    if(type_contains_lambda_mangle_metadata(type->params[i])) {
      return true;
    }
  }

  return false;
}

static bool type_has_structured_dependent_qualified_member(const TypePtr & type)
{
  if(!type) {
    return false;
  }

  if(type->kind == Type::TK_NAMED) {
    TypePtr owner;
    vector<string> members;
    bool leading_typename = false;
    if(named_type_dependent_qualified_member(type,
                                             owner,
                                             members,
                                             leading_typename)) {
      (void)leading_typename;
      return true;
    }
  }

  if(type_has_structured_dependent_qualified_member(type->owner) ||
     type_has_structured_dependent_qualified_member(type->inner)) {
    return true;
  }

  for(size_t i = 0; i < type->params.size(); ++i) {
    if(type_has_structured_dependent_qualified_member(type->params[i])) {
      return true;
    }
  }

  return false;
}

static bool type_has_dependent_class_template_nested_owner_mangle_state(
    const TypePtr & type)
{
  if(!type) {
    return false;
  }

  if(type->kind == Type::TK_NAMED) {
    void * class_template_decl = nullptr;
    vector<DependentAliasTemplateArgumentSyntax> arguments;
    shared_ptr<const ClassTemplateSpecializationMangleInfo> owner_info =
        named_type_class_template_specialization_mangle_info_const(type);
    if(named_type_dependent_class_template(type,
                                           class_template_decl,
                                           arguments) &&
       owner_info &&
       count_scope_operators(owner_info->template_scope_prefix) > 0) {
      return true;
    }
  }

  if(type_has_dependent_class_template_nested_owner_mangle_state(type->owner) ||
     type_has_dependent_class_template_nested_owner_mangle_state(type->inner)) {
    return true;
  }

  for(size_t i = 0; i < type->params.size(); ++i) {
    if(type_has_dependent_class_template_nested_owner_mangle_state(type->params[i])) {
      return true;
    }
  }

  return false;
}

static bool try_mangle_template_argument_impl(const TemplateArgument & arg,
                                              const TemplateParameterInfo * parameter,
                                              string & out,
                                              const TypeMangleContext * mangle_ctx,
                                              MangleSubstitutionState * state,
                                              size_t parameter_index)
{
  switch(arg.kind) {
  case TemplateArgument::TA_TYPE: {
    if(arg.source_syntax &&
       !arg.source_defaulted &&
       !type_contains_lambda_mangle_metadata(arg.type) &&
       (arg.dependent ||
        type_has_dependent_mangle_state(arg.type))) {
      const size_t begin = out.size();
      MangleSubstitutionCheckpoint checkpoint(state);
      if(try_mangle_template_argument_syntax_impl(
             *arg.source_syntax,
             parameter,
             out,
             mangle_ctx,
             state)) {
        checkpoint.commit();
        return true;
      }
      out.resize(begin);
    }
    abi_mangle::Type ir_type;
    if(try_build_type_ir_cached(arg.type, mangle_ctx, ir_type)) {
      MangleIrSubstitutionSink sink(state);
      if(!abi_mangle::emit_template_argument(
             abi_mangle::TemplateArgument::type_arg(ir_type),
             out,
             &sink)) {
        return false;
      }
    } else if(!try_emit_type_encoding_ir_impl(arg.type, out, mangle_ctx, state)) {
      return false;
    }
    register_direct_std_type_substitution_if_needed(arg.type, mangle_ctx, state);
    return true;
  }

  case TemplateArgument::TA_VALUE:
  {
    const bool prefer_concrete_dependent_parameter_value =
        parameter &&
        !arg.dependent &&
        template_value_argument_prefers_concrete_dependent_parameter_value(
            arg,
            parameter,
            mangle_ctx);
    if(parameter &&
       !prefer_concrete_dependent_parameter_value &&
       try_mangle_dependent_non_type_template_argument(
           arg, *parameter, out, mangle_ctx, state, parameter_index)) {
      return true;
    }
    if(prefer_concrete_dependent_parameter_value) {
      if(arg.type &&
         try_mangle_value_template_argument_impl(arg.type,
                                                 arg.value,
                                                 out,
                                                 state)) {
        return true;
      }
      return try_mangle_untyped_integral_template_argument_value(arg.value, out);
    }
    ExternalEntityArgumentIrPayload entity_payload;
    if(try_build_external_entity_argument_ir_payload(arg,
                                                     mangle_ctx,
                                                     entity_payload)) {
      MangleIrSubstitutionSink sink(state);
      return abi_mangle::emit_template_argument(
          function_external_entity_argument_from_payload(entity_payload),
          out,
          &sink);
    }
    if(!arg.dependent && (arg.type || (parameter && parameter->value_type))) {
      TypePtr value_type =
          parameter && parameter->value_type ? parameter->value_type : arg.type;
      if(arg.type &&
         value_type &&
         type_has_dependent_mangle_state(value_type) &&
         !type_has_dependent_mangle_state(arg.type)) {
        value_type = arg.type;
      }
      TypePtr value_base = strip_top_level_cv(value_type);
      if(arg.type &&
         value_base &&
         value_base->kind == Type::TK_NAMED &&
         (named_type_has_dependent_semantic(value_base) ||
          value_base->named_display.find("::") != string::npos ||
          value_base->named_key.find("::") != string::npos)) {
        value_type = arg.type;
      }
      if(!arg.type &&
         value_base &&
         value_base->kind == Type::TK_NAMED &&
         (named_type_has_dependent_semantic(value_base) ||
          value_base->named_display.find("::") != string::npos ||
          value_base->named_key.find("::") != string::npos)) {
        return try_mangle_untyped_integral_template_argument_value(arg.value, out);
      }
      return try_mangle_value_template_argument_impl(value_type,
                                                     arg.value,
                                                     out,
                                                     state);
    }
    if(arg.dependent && arg.expression) {
      return try_mangle_dependent_expression_ast_template_argument(
          *arg.expression, out, mangle_ctx, state);
    }
    if(arg.dependent && !arg.text.empty()) {
      if(try_mangle_non_type_template_parameter_expression_pack_text(arg.text,
                                                                      out,
                                                                      mangle_ctx,
                                                                      state)) {
        return true;
      }
      if(try_mangle_non_type_template_parameter_expression_name(arg.text,
                                                                out,
                                                                mangle_ctx,
                                                                state)) {
        return true;
      }
      if(parameter &&
         parameter_index != static_cast<size_t>(-1) &&
         !parameter->name.empty() &&
         trim_elaborated_type_prefix(arg.text) == parameter->name) {
        return emit_template_parameter_index(parameter_index,
                                             *parameter,
                                             out,
                                             state,
                                             false);
      }
      throw_unstructured_dependent_text_mangling(
          "non-type template argument",
          arg.text);
    }
    return try_mangle_value_template_argument_impl(TypePtr(), arg.value, out, state);
  }

  case TemplateArgument::TA_CLASS_TEMPLATE:
  case TemplateArgument::TA_ALIAS_TEMPLATE:
  {
    vector<abi_mangle::Type::NameComponent> prefix_components;
    string template_name;
    string template_name_substitution;
    abi_mangle::Type template_owner;
    if(try_build_template_entity_argument_name_ir(arg,
                                                  prefix_components,
                                                  template_name,
                                                  template_name_substitution,
                                                  mangle_ctx,
                                                  &template_owner)) {
      MangleIrSubstitutionSink sink(state);
	      abi_mangle::TemplateArgument argument =
	          template_owner.kind == abi_mangle::Type::TK_INVALID ?
	              abi_mangle::TemplateArgument::template_entity_arg(
	                  std::move(prefix_components),
	                  std::move(template_name),
	                  std::move(template_name_substitution)) :
	              abi_mangle::TemplateArgument::member_template_entity_arg(
	                  std::move(template_owner),
	                  std::move(template_name),
	                  std::move(template_name_substitution));
      return abi_mangle::emit_template_argument(argument, out, &sink);
    }
    if(arg.text.empty()) {
      return false;
    }
    if(try_mangle_template_parameter_text(arg.text, mangle_ctx, out, state)) {
      return true;
    }
    return false;
  }
  }
  return false;
}

static bool try_mangle_builtin_text(const string & trimmed, string & out)
{
  static const struct {
    const char * text;
    const char * code;
  } kBuiltinArgs[] = {
      {"signed char", "a"},
      {"short", "s"},
      {"short int", "s"},
      {"signed short", "s"},
      {"signed short int", "s"},
      {"int", "i"},
      {"signed", "i"},
      {"signed int", "i"},
      {"long", "l"},
      {"long int", "l"},
      {"signed long", "l"},
      {"signed long int", "l"},
      {"long long", "x"},
      {"long long int", "x"},
      {"signed long long", "x"},
      {"signed long long int", "x"},
      {"unsigned char", "h"},
      {"unsigned short", "t"},
      {"unsigned short int", "t"},
      {"unsigned int", "j"},
      {"unsigned", "j"},
      {"unsigned long", "m"},
      {"unsigned long int", "m"},
      {"unsigned long long", "y"},
      {"unsigned long long int", "y"},
      {"wchar_t", "w"},
      {"char", "c"},
      {"bool", "b"},
      {"float", "f"},
      {"double", "d"},
      {"long double", "e"},
      {"void", "v"},
      {"char16_t", "Ds"},
      {"char32_t", "Di"},
      {"decltype(nullptr)", "Dn"}};
  for(size_t i = 0; i < sizeof(kBuiltinArgs) / sizeof(kBuiltinArgs[0]); ++i) {
    if(trimmed == kBuiltinArgs[i].text) {
      out += kBuiltinArgs[i].code;
      return true;
    }
  }
  return false;
}

static string preferred_named_type_text(const TypePtr & type,
                                        const TypeMangleContext * mangle_ctx)
{
  if(!type || type->kind != Type::TK_NAMED) {
    return string();
  }

  const string display_text = trim_elaborated_type_prefix(type->named_display);
  const string key_text = trim_elaborated_type_prefix(type->named_key);
  string selected_text = display_text;
  const bool prefer_lexical_name =
      should_prefer_unqualified_lexical_named_type(type, mangle_ctx);
  if(!prefer_lexical_name &&
     named_type_should_prefer_key_for_mangling(*type, display_text, key_text)) {
    selected_text = key_text;
  }
  if(prefer_lexical_name) {
    selected_text =
        append_qualified_component_text(mangle_ctx->lexical_scope, display_text);
  }

  string canonical_text;
  if(canonicalize_named_substitution_text(selected_text, canonical_text) &&
     !canonical_text.empty()) {
    return canonical_text;
  }
  return selected_text;
}

static vector<const Type *> & type_substitution_key_recursion_stack()
{
  static vector<const Type *> stack;
  return stack;
}

struct TypeSubstitutionKeyRecursionGuard
{
  explicit TypeSubstitutionKeyRecursionGuard(const Type * type)
      : stack(type_substitution_key_recursion_stack()),
        active(std::find(stack.begin(), stack.end(), type) == stack.end())
  {
    if(active) {
      stack.push_back(type);
    }
  }

  ~TypeSubstitutionKeyRecursionGuard()
  {
    if(active) {
      stack.pop_back();
    }
  }

  bool entered() const { return active; }

private:
  vector<const Type *> & stack;
  bool active;
};

static bool build_type_substitution_key_impl(const TypePtr & type,
                                             const TypeMangleContext * mangle_ctx,
                                             string & out,
                                             bool allow_fundamental_atom)
{
  if(!type) {
    return false;
  }
  TypeSubstitutionKeyRecursionGuard recursion_guard(type.get());
  if(!recursion_guard.entered()) {
    return false;
  }
  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    return allow_fundamental_atom &&
           build_fundamental_substitution_atom_key(type->fundamental, out);

  case Type::TK_CV: {
    string inner_key;
    if(!build_type_substitution_key_impl(type->inner, mangle_ctx, inner_key, true) ||
       inner_key.empty()) {
      return false;
    }
    out = string("type:cv(");
    if(type->cv_const) {
      out += 'K';
    }
    if(type->cv_volatile) {
      out += 'V';
    }
    out += ",";
    out += inner_key;
    out += ")";
    return true;
  }

  case Type::TK_POINTER: {
    string inner_key;
    if(!build_type_substitution_key_impl(type->inner, mangle_ctx, inner_key, true) ||
       inner_key.empty()) {
      return false;
    }
    out = string("type:ptr(") + inner_key + ")";
    return true;
  }

  case Type::TK_LVALUE_REFERENCE: {
    string inner_key;
    if(!build_type_substitution_key_impl(type->inner, mangle_ctx, inner_key, true) ||
       inner_key.empty()) {
      return false;
    }
    out = string("type:lref(") + inner_key + ")";
    return true;
  }

  case Type::TK_RVALUE_REFERENCE: {
    string inner_key;
    if(!build_type_substitution_key_impl(type->inner, mangle_ctx, inner_key, true) ||
       inner_key.empty()) {
      return false;
    }
    out = string("type:rref(") + inner_key + ")";
    return true;
  }

  case Type::TK_ARRAY: {
    string inner_key;
    if(!build_type_substitution_key_impl(type->inner, mangle_ctx, inner_key, true) ||
       inner_key.empty()) {
      return false;
    }
    string bound_key;
    if(type->has_bound) {
      bound_key = string("expr:int(") + to_string(type->bound) + ")";
    } else if(type->bound_text.empty()) {
      bound_key = "expr:unknown";
    } else if(!build_array_bound_text_key(type->bound_text, bound_key) &&
              !build_template_parameter_index_expression_key(
                  type->bound_text, mangle_ctx, bound_key)) {
      return false;
    }
    out = string("type:array(") + bound_key + "," + inner_key + ")";
    return true;
  }

  case Type::TK_FUNCTION: {
    string return_key;
    if(!type->inner ||
       !build_type_substitution_key_impl(type->inner, mangle_ctx, return_key, true) ||
       return_key.empty()) {
      return false;
    }
    ostringstream key;
    key << "type:fn(" << return_key << ";";
    for(size_t i = 0; i < type->params.size(); ++i) {
      string param_key;
      if(!build_type_substitution_key_impl(type->params[i], mangle_ctx, param_key, true) ||
         param_key.empty()) {
        return false;
      }
      if(i != 0) {
        key << ",";
      }
      key << param_key;
    }
    key << ";";
    key << (type->variadic ? "z" : "v");
    key << ")";
    out = key.str();
    return true;
  }

  case Type::TK_MEMBER_POINTER: {
    string owner_key;
    string member_key;
    if(!type->owner ||
       !type->inner ||
       !build_type_substitution_key_impl(type->owner, mangle_ctx, owner_key, true) ||
       !build_type_substitution_key_impl(type->inner, mangle_ctx, member_key, true) ||
       owner_key.empty() ||
       member_key.empty()) {
      return false;
    }
    out = string("type:mptr(") + owner_key + "," + member_key + ")";
    return true;
  }

  case Type::TK_ATOMIC: {
    string inner_key;
    if(!build_type_substitution_key_impl(type->inner, mangle_ctx, inner_key, true) ||
       inner_key.empty()) {
      return false;
    }
    out = string("type:vendor(_Atomic,") + inner_key + ")";
    return true;
  }

  case Type::TK_NAMED: {
    const string named_display = trim_elaborated_type_prefix(type->named_display);
    const bool has_template_parameter_key =
        type->named_semantic_kind == Type::NSK_TEMPLATE_PARAMETER;
    if(mangle_ctx && mangle_ctx->template_parameters &&
       mangle_ctx->template_parameters->parameters) {
      for(size_t i = 0; i < mangle_ctx->template_parameters->parameters->size(); ++i) {
        const TemplateParameterInfo & param = (*mangle_ctx->template_parameters->parameters)[i];
        if(param.kind != TemplateParameterInfo::TP_TYPE) {
          continue;
        }
        if((!param.placeholder_key.empty() && type->named_key == param.placeholder_key) ||
           (has_template_parameter_key &&
            !param.name.empty() &&
            text_matches_type_parameter_name(named_display, param.name))) {
          out = template_parameter_type_substitution_key(
              mangle_ctx->template_parameters->parameters, i, param);
          return true;
        }
      }
    }
    if(mangle_ctx &&
       mangle_ctx->owner_template_parameters &&
       mangle_ctx->owner_template_arguments) {
      const vector<TemplateParameterInfo> & parameters =
          *mangle_ctx->owner_template_parameters;
      const vector<TemplateArgument> & arguments =
          *mangle_ctx->owner_template_arguments;
      for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
        const TemplateParameterInfo & param = parameters[i];
        if(param.kind != TemplateParameterInfo::TP_TYPE || param.name.empty()) {
          continue;
        }
        if((!param.placeholder_key.empty() && type->named_key == param.placeholder_key) ||
           (has_template_parameter_key &&
            text_matches_type_parameter_name(named_display, param.name))) {
          if(owner_template_argument_index_is_suppressed(mangle_ctx, i)) {
            out = template_parameter_type_substitution_key(&parameters, i, param);
            return true;
          }
          TypePtr argument_type = arguments[i].type;
          TypePtr argument_base = strip_top_level_cv(argument_type);
          if(!argument_base ||
             argument_base.get() == type.get() ||
             (argument_base->kind == Type::TK_NAMED &&
              argument_base->named_key == type->named_key &&
              trim_elaborated_type_prefix(
                  argument_base->named_display) == named_display)) {
            out = template_parameter_type_substitution_key(&parameters, i, param);
            return true;
          }
          TypeMangleContext owner_arg_ctx_storage;
          return build_type_substitution_key_impl(
              argument_type,
              suppress_owner_template_argument_index(
                  mangle_ctx, i, owner_arg_ctx_storage),
              out,
              allow_fundamental_atom);
        }
      }
    }
    out = named_substitution_key(preferred_named_type_text(type, mangle_ctx));
    return !out.empty();
  }

  case Type::TK_BLOCK_POINTER:
    return false;
  }

  return false;
}

static bool build_type_substitution_key(const TypePtr & type,
                                        const TypeMangleContext * mangle_ctx,
                                        string & out)
{
  return build_type_substitution_key_impl(type, mangle_ctx, out, false);
}

static bool build_structural_type_substitution_key(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::SubstitutionKey & out,
    bool allow_fundamental_atom);

static bool build_class_template_specialization_structural_key(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::SubstitutionKey & out)
{
  shared_ptr<const ClassTemplateSpecializationMangleInfo> info =
      named_type_class_template_specialization_mangle_info_const(type);
  if(!info) {
    return false;
  }
  const TypeMangleContext * key_ctx = mangle_ctx;

  string canonical_scope;
  if(!canonicalize_named_substitution_text(info->template_scope_prefix,
                                           canonical_scope)) {
    canonical_scope = trim_space(info->template_scope_prefix);
  }
  const string template_name = trim_space(info->template_name);
  if(template_name.empty()) {
    return false;
  }
  const string fallback_name =
      append_qualified_component_text(canonical_scope,
                                      canonical_component_text(template_name));

  const vector<TemplateParameterInfo> * parameters =
      info->template_parameters.empty() ? nullptr : &info->template_parameters;
  vector<abi_mangle::Type::ClassTemplateArgument> arguments;
  if(!try_build_class_template_arguments_ir(info->arguments,
                                            parameters,
                                            nullptr,
                                            key_ctx,
                                            arguments)) {
    return false;
  }

  vector<abi_mangle::SubstitutionKey> argument_keys;
  argument_keys.reserve(arguments.size());
  for(size_t i = 0; i < arguments.size(); ++i) {
    abi_mangle::SubstitutionKey argument_key;
    if(!abi_mangle::make_class_template_argument_substitution_key(
           arguments[i],
           argument_key)) {
      return false;
    }
    argument_keys.push_back(std::move(argument_key));
  }

  out = abi_mangle::SubstitutionKey::class_template_specialization(
      0,
      fallback_name,
      std::move(argument_keys));
  return !out.empty();
}

static bool build_structural_type_substitution_key(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::SubstitutionKey & out,
    bool allow_fundamental_atom)
{
  if(!type) {
    return false;
  }

  switch(type->kind) {
  case Type::TK_FUNDAMENTAL: {
    string code;
    if(!allow_fundamental_atom ||
       !append_fundamental_mangle_code(type->fundamental, code) ||
       code.empty()) {
      return false;
    }
    out = abi_mangle::SubstitutionKey::type_builtin(code);
    return true;
  }

  case Type::TK_CV: {
    abi_mangle::SubstitutionKey inner_key;
    if(!build_structural_type_substitution_key(type->inner,
                                               mangle_ctx,
                                               inner_key,
                                               true)) {
      return false;
    }
    out = abi_mangle::SubstitutionKey::type_cv(type->cv_const,
                                                      type->cv_volatile,
                                                      std::move(inner_key));
    return true;
  }

  case Type::TK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE: {
    abi_mangle::SubstitutionKey inner_key;
    if(!build_structural_type_substitution_key(type->inner,
                                               mangle_ctx,
                                               inner_key,
                                               true)) {
      return false;
    }
    if(type->kind == Type::TK_POINTER) {
      out = abi_mangle::SubstitutionKey::type_pointer(std::move(inner_key));
    } else if(type->kind == Type::TK_LVALUE_REFERENCE) {
      out = abi_mangle::SubstitutionKey::type_lvalue_reference(
          std::move(inner_key));
    } else {
      out = abi_mangle::SubstitutionKey::type_rvalue_reference(
          std::move(inner_key));
    }
    return true;
  }

  case Type::TK_ARRAY: {
    string bound_key;
    if(!build_array_bound_substitution_key(type, mangle_ctx, bound_key)) {
      return false;
    }
    abi_mangle::SubstitutionKey inner_key;
    if(!build_structural_type_substitution_key(type->inner,
                                               mangle_ctx,
                                               inner_key,
                                               true)) {
      return false;
    }
    out = abi_mangle::SubstitutionKey::type_array(bound_key,
                                                  std::move(inner_key));
    return true;
  }

  case Type::TK_FUNCTION: {
    abi_mangle::SubstitutionKey result_key;
    if(!build_structural_type_substitution_key(type->inner,
                                               mangle_ctx,
                                               result_key,
                                               true)) {
      return false;
    }
    vector<abi_mangle::SubstitutionKey> param_keys;
    param_keys.reserve(type->params.size());
    for(size_t i = 0; i < type->params.size(); ++i) {
      abi_mangle::SubstitutionKey param_key;
      if(!build_structural_type_substitution_key(type->params[i],
                                                 mangle_ctx,
                                                 param_key,
                                                 true)) {
        return false;
      }
      param_keys.push_back(std::move(param_key));
    }
    out = abi_mangle::SubstitutionKey::type_function(std::move(result_key),
                                                     std::move(param_keys),
                                                     type->variadic,
                                                     type->function_ref_qualifier == FTRQ_LVALUE,
                                                     type->function_ref_qualifier == FTRQ_RVALUE);
    apply_function_type_cv_substitution_key(type, out);
    return true;
  }

  case Type::TK_MEMBER_POINTER: {
    abi_mangle::SubstitutionKey owner_key;
    abi_mangle::SubstitutionKey member_key;
    if(!build_structural_type_substitution_key(type->owner,
                                               mangle_ctx,
                                               owner_key,
                                               true) ||
       !build_structural_type_substitution_key(type->inner,
                                               mangle_ctx,
                                               member_key,
                                               true)) {
      return false;
    }
    out = abi_mangle::SubstitutionKey::type_member_pointer(
        std::move(owner_key),
        std::move(member_key));
    return true;
  }

  case Type::TK_ATOMIC: {
    abi_mangle::SubstitutionKey inner_key;
    if(!build_structural_type_substitution_key(type->inner,
                                               mangle_ctx,
                                               inner_key,
                                               true)) {
      return false;
    }
    out = abi_mangle::SubstitutionKey::type(
        string("vendor-qualified:_Atomic:") + inner_key.structural_text());
    return true;
  }

  case Type::TK_NAMED:
    if(build_class_template_specialization_structural_key(type, mangle_ctx, out)) {
      return true;
    }
    out = abi_mangle::SubstitutionKey::named(
        preferred_named_type_text(type, mangle_ctx));
    return !out.empty();

  case Type::TK_BLOCK_POINTER:
    return false;
  }
  return false;
}

static bool type_names_match_qualified_owner(const TypePtr & type,
                                             const string & owner_text)
{
  if(!type) {
    return false;
  }
  if(type->kind == Type::TK_CV) {
    return type_names_match_qualified_owner(type->inner, owner_text);
  }
  if(type->kind == Type::TK_POINTER || type->kind == Type::TK_LVALUE_REFERENCE ||
     type->kind == Type::TK_RVALUE_REFERENCE) {
    return type_names_match_qualified_owner(type->inner, owner_text);
  }
  if(type->kind != Type::TK_NAMED) {
    return false;
  }
  const string normalized_owner =
      remove_space_chars(trim_elaborated_type_prefix(owner_text));
  const string normalized_display =
      remove_space_chars(trim_elaborated_type_prefix(type->named_display));
  if(normalized_display == normalized_owner) {
    return true;
  }
  const string normalized_key =
      remove_space_chars(trim_elaborated_type_prefix(type->named_key));
  return !normalized_key.empty() && normalized_key == normalized_owner;
}

static string qualified_prefix_text(const QualifiedName & qualified, size_t count)
{
  string out;
  for(size_t i = 0; i < count; ++i) {
    if(i != 0) {
      out += "::";
    }
    out += qualified.qualifiers[i];
  }
  return out;
}

static string qualified_prefix_text(const QualifiedName & qualified)
{
  return qualified_prefix_text(qualified, qualified.qualifiers.size());
}

static string function_type_lexical_scope(const QualifiedName & qualified,
                                          const FunctionSymbolOptions & options)
{
  size_t count = qualified.qualifiers.size();
  if(options.is_member_function && count > 0) {
    --count;
  }
  return qualified_prefix_text(qualified, count);
}

static QualifiedName function_type_lexical_scope_syntax(
    const QualifiedName & qualified,
    const FunctionSymbolOptions & options)
{
  size_t count = qualified.qualifiers.size();
  if(options.is_member_function && count > 0) {
    --count;
  }
  QualifiedName out;
  out.rooted = qualified.rooted;
  if(count == 0) {
    return out;
  }
  out.qualifiers.assign(qualified.qualifiers.begin(),
                        qualified.qualifiers.begin() + count - 1);
  out.name = qualified.qualifiers[count - 1];
  return out;
}

static string compact_operator_name(const string & text)
{
  return remove_space_chars(text);
}

static bool name_starts_with_operator_text(const string & text)
{
  return text.size() >= 8 && text.compare(0, 8, "operator") == 0;
}

static size_t operator_explicit_parameter_count(const QualifiedName & qualified,
                                                const TypePtr & type,
                                                const FunctionSymbolOptions & options)
{
  if(!type || type->kind != Type::TK_FUNCTION) {
    return 0;
  }
  size_t count = type->params.size();
  if(!options.has_implicit_object_parameter || count == 0) {
    return count;
  }

  if(qualified.qualifiers.empty()) {
    return count;
  }

  const string owner_text = qualified_prefix_text(qualified);
  if(type_names_match_qualified_owner(type->params[0], owner_text)) {
    --count;
  }
  return count;
}

static bool fixed_operator_mangle_terminal(
    const string & compact,
    size_t explicit_param_count,
    bool is_member_function,
    abi_mangle::FunctionOperatorTerminal & terminal,
    string & literal_suffix)
{
  if(!abi_mangle::function_operator_terminal_from_cpp_spelling(
         compact,
         explicit_param_count,
         is_member_function,
         terminal,
         literal_suffix)) {
    return false;
  }
  return terminal != abi_mangle::FUNCTION_OPERATOR_LITERAL ||
         is_identifier_text_for_mangling(literal_suffix);
}

static bool template_value_argument_needs_entity_identity(
    const TemplateArgument & argument)
{
  if(argument.kind != TemplateArgument::TA_VALUE ||
     argument.dependent ||
     !argument.type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(argument.type);
  return base &&
         !argument.function_value &&
         !argument.value_binding &&
         (base->kind == Type::TK_POINTER ||
          base->kind == Type::TK_LVALUE_REFERENCE ||
          base->kind == Type::TK_RVALUE_REFERENCE ||
          base->kind == Type::TK_MEMBER_POINTER ||
          base->kind == Type::TK_BLOCK_POINTER);
}

static bool owner_template_arguments_can_use_structured_mangling(
    const vector<TemplateArgument> & arguments)
{
  for(size_t i = 0; i < arguments.size(); ++i) {
    if(template_value_argument_needs_entity_identity(arguments[i])) {
      return false;
    }
  }
  return true;
}

static const FunctionSymbolOptions::OwnerTemplateComponent *
find_matching_owner_template_component(const FunctionSymbolOptions & options,
                                       const string & base_name,
                                       size_t & search_index)
{
  const string stripped = trim_space(base_name);
  if(stripped.empty()) {
    return nullptr;
  }
  for(size_t i = search_index; i < options.owner_template_components.size(); ++i) {
    const FunctionSymbolOptions::OwnerTemplateComponent & component =
        options.owner_template_components[i];
    if(trim_space(component.template_name) != stripped ||
       !component.arguments ||
       !owner_template_arguments_can_use_structured_mangling(
           *component.arguments)) {
      continue;
    }
    search_index = i + 1;
    return &component;
  }
  return nullptr;
}

static string function_template_prefix_substitution_key(
    const vector<string> & qualifiers,
    const string & name)
{
  vector<string> parts;
  parts.reserve(qualifiers.size() + 1);
  for(size_t i = 0; i < qualifiers.size(); ++i) {
    parts.push_back(canonical_component_text(qualifiers[i]));
  }
  parts.push_back(canonical_component_text(name));
  return string("function-template-prefix:") +
         join_canonical_qualified_parts(parts, parts.size());
}

static vector<FunctionParameterMangleInfo> build_function_parameter_mangle_context(
    const FunctionSymbolOptions & options)
{
  vector<FunctionParameterMangleInfo> out;
  if(options.parameter_pattern) {
    out.resize(options.parameter_pattern->size());
    for(size_t i = 0; i < options.parameter_pattern->size(); ++i) {
      out[i].name = (*options.parameter_pattern)[i].first;
    }
  }

  if(options.parameter_declarations_pattern) {
    if(out.size() < options.parameter_declarations_pattern->size()) {
      out.resize(options.parameter_declarations_pattern->size());
    }
    for(size_t i = 0; i < options.parameter_declarations_pattern->size(); ++i) {
      const CppAstNode * parameter = (*options.parameter_declarations_pattern)[i];
      if(!parameter) {
        continue;
      }
      const string name =
          pack_parameter_analysis::parameter_declaration_name(*parameter);
      if(!name.empty()) {
        out[i].name = name;
      }
    }
  }

  return out;
}

static bool build_function_template_argument_ir_substitution_keys(
    const vector<abi_mangle::TemplateArgument> & arguments,
    vector<abi_mangle::SubstitutionKey> & out)
{
  out.clear();
  out.reserve(arguments.size());
  for(size_t i = 0; i < arguments.size(); ++i) {
    abi_mangle::SubstitutionKey key;
    if(!abi_mangle::make_template_argument_substitution_key(arguments[i],
                                                                   key)) {
      out.clear();
      return false;
    }
    out.push_back(std::move(key));
  }
  return true;
}

static bool build_class_template_argument_ir_substitution_keys(
    const vector<abi_mangle::Type::ClassTemplateArgument> & arguments,
    vector<abi_mangle::SubstitutionKey> & out)
{
  out.clear();
  out.reserve(arguments.size());
  for(size_t i = 0; i < arguments.size(); ++i) {
    abi_mangle::SubstitutionKey key;
    if(!abi_mangle::make_class_template_argument_substitution_key(
           arguments[i],
           key)) {
      out.clear();
      return false;
    }
    out.push_back(std::move(key));
  }
  return true;
}

static bool function_options_are_plain_ir_candidate(
    const FunctionSymbolOptions & options)
{
  return !options.is_member_function &&
         !options.has_implicit_object_parameter &&
         !options.is_const_method &&
         !options.is_volatile_method &&
         options.ref_qualifier == FRQ_NONE &&
         !options.is_constructor &&
         !options.is_destructor &&
         !options.is_conversion_operator &&
         !options.function_type_pattern &&
         !options.template_parameters &&
         (!options.template_arguments || options.template_arguments->empty()) &&
         !options.template_argument_pack_sizes &&
         !options.owner_template_parameters &&
         !options.owner_template_arguments &&
         options.owner_template_name.empty() &&
         options.owner_template_components.empty() &&
         !options.parameter_pattern &&
         !options.result_type_pattern &&
         !options.parameter_declarations_pattern &&
         !options.has_trailing_function_parameter_pack &&
         !options.suppress_template_argument_pack_grouping &&
         !options.lambda_closure_type &&
         !options.local_class_type;
}

static bool function_options_are_simple_name_ir_candidate(
    const FunctionSymbolOptions & options)
{
  return !options.is_member_function &&
         !options.has_implicit_object_parameter &&
         !options.is_const_method &&
         !options.is_volatile_method &&
         options.ref_qualifier == FRQ_NONE &&
         !options.is_constructor &&
         !options.is_destructor &&
         !options.is_conversion_operator &&
         !options.lambda_closure_type &&
         !options.local_class_type;
}

static bool function_options_are_simple_member_name_ir_candidate(
    const FunctionSymbolOptions & options)
{
  return options.is_member_function &&
         !options.is_constructor &&
         !options.is_destructor &&
         !options.is_conversion_operator &&
         !options.lambda_closure_type &&
         !options.local_class_type;
}

static bool function_options_are_fixed_operator_name_ir_candidate(
    const FunctionSymbolOptions & options)
{
  if(options.is_constructor ||
     options.is_destructor ||
     options.lambda_closure_type ||
     options.local_class_type) {
    return false;
  }
  if(options.is_member_function) {
    return true;
  }
  return !options.is_const_method &&
         !options.is_volatile_method &&
         options.ref_qualifier == FRQ_NONE;
}

static bool ordinary_identifier_operator_prefix_name(
    const QualifiedName & qualified,
    const string & display_name)
{
  return (display_name.empty() || display_name == qualified.name) &&
         is_identifier_text_for_mangling(qualified.name);
}

static bool function_name_requires_operator_ir_path(
    const QualifiedName & qualified,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options)
{
  const string & display_or_name =
      display_name.empty() ? qualified.name : display_name;
  if(!name_starts_with_operator_text(display_or_name)) {
    return false;
  }
  const string compact = compact_operator_name(display_or_name);
  if(compact.compare(0, 8, "operator") != 0) {
    return false;
  }
  abi_mangle::FunctionOperatorTerminal fixed_terminal =
      abi_mangle::FUNCTION_OPERATOR_NONE;
  string fixed_literal_suffix;
  if(fixed_operator_mangle_terminal(
         compact,
         operator_explicit_parameter_count(qualified, type, options),
         options.is_member_function,
         fixed_terminal,
         fixed_literal_suffix)) {
    return true;
  }
  if(options.is_member_function &&
     !qualified.qualifiers.empty() &&
     options.is_conversion_operator) {
    return true;
  }
  return false;
}

static void apply_member_function_nested_qualifiers_ir(
    const FunctionSymbolOptions & options,
    abi_mangle::FunctionEncoding & function)
{
  function.nested_const = options.is_const_method;
  function.nested_volatile = options.is_volatile_method;
  function.nested_lvalue_ref = options.ref_qualifier == FRQ_LVALUE;
  function.nested_rvalue_ref = options.ref_qualifier == FRQ_RVALUE;
}

static void initialize_function_template_argument_mangle_context(
    const QualifiedName & qualified,
    const FunctionSymbolOptions & options,
    TemplateParameterMangleContext & template_argument_parameter_ctx,
    TypeMangleContext & template_argument_mangle_ctx)
{
  if(options.template_parameters) {
    template_argument_parameter_ctx.parameters = options.template_parameters;
    template_argument_mangle_ctx.template_parameters =
        &template_argument_parameter_ctx;
  }
  template_argument_mangle_ctx.owner_template_parameters =
      options.owner_template_parameters;
  template_argument_mangle_ctx.owner_template_arguments =
      options.owner_template_arguments;
  template_argument_mangle_ctx.lookup_scope = options.lookup_scope;
  template_argument_mangle_ctx.lexical_scope =
      function_type_lexical_scope(qualified, options);
  template_argument_mangle_ctx.lexical_scope_syntax =
      function_type_lexical_scope_syntax(qualified, options);
  template_argument_mangle_ctx.suppress_template_argument_pack_grouping =
      options.suppress_template_argument_pack_grouping;
  template_argument_mangle_ctx.template_argument_pack_sizes =
      options.template_argument_pack_sizes;
}

static bool try_build_function_qualifier_component_ir(
    const string & part,
    const FunctionSymbolOptions & options,
    bool is_last_qualifier,
    string & canonical_prefix,
    size_t & owner_template_component_index,
    abi_mangle::FunctionNameComponent & out)
{
  TemplateComponent component;
  const bool has_parsed_component = parse_template_component(part, component);
  TypeMangleContext owner_component_ctx;
  owner_component_ctx.lookup_scope = options.lookup_scope;
  owner_component_ctx.suppress_template_argument_pack_grouping =
      options.suppress_template_argument_pack_grouping;
  owner_component_ctx.prefer_concrete_non_type_values_for_dependent_parameter_types = true;
  size_t candidate_owner_template_component_index =
      owner_template_component_index;
  const FunctionSymbolOptions::OwnerTemplateComponent * owner_component =
      has_parsed_component ?
          find_matching_owner_template_component(
              options,
              component.base_name,
              candidate_owner_template_component_index) :
          nullptr;
  bool component_matches_owner_template =
      !options.owner_template_name.empty() &&
      has_parsed_component &&
      trim_space(component.base_name) == options.owner_template_name;
  const vector<TemplateParameterInfo> * structured_parameters =
      owner_component ? owner_component->parameters : options.owner_template_parameters;
  const vector<TemplateArgument> * structured_arguments =
      owner_component ? owner_component->arguments : options.owner_template_arguments;
  vector<TemplateArgument> structured_arguments_with_source;
  if(owner_component &&
     owner_component->argument_syntaxes &&
     structured_arguments &&
     owner_component->argument_syntaxes->size() == structured_arguments->size()) {
    structured_arguments_with_source = *structured_arguments;
    for(size_t i = 0; i < structured_arguments_with_source.size(); ++i) {
      if(!structured_arguments_with_source[i].source_syntax) {
        structured_arguments_with_source[i].source_syntax.reset(
            new TemplateArgumentSyntax((*owner_component->argument_syntaxes)[i]));
      }
    }
    structured_arguments = &structured_arguments_with_source;
  }
  if(owner_component) {
    component_matches_owner_template = true;
  }
  const semantic_model::ClassInfo * lexical_owner = nullptr;
  for(const semantic_model::Scope * current = options.lookup_scope;
      current && !lexical_owner;
      current = current->parent) {
    lexical_owner = current->class_info;
  }
  const bool component_matches_lexical_owner_template =
      !owner_component &&
      is_last_qualifier &&
      has_parsed_component &&
      lexical_owner &&
      lexical_owner->source_template &&
      trim_space(component.base_name) == lexical_owner->source_template->name;
  if(component_matches_lexical_owner_template) {
    component_matches_owner_template = true;
    structured_parameters = &lexical_owner->source_template->parameters;
    structured_arguments = &lexical_owner->instantiation_arguments;
  }
  owner_component_ctx.owner_template_parameters = structured_parameters;
  owner_component_ctx.owner_template_arguments = structured_arguments;
  TemplateParameterMangleContext owner_component_template_parameter_ctx;
  if(owner_component && owner_component->mangle_parameters) {
    owner_component_template_parameter_ctx.parameters =
        owner_component->mangle_parameters;
    owner_component_ctx.template_parameters =
        &owner_component_template_parameter_ctx;
  }
  const bool should_use_structured_owner_arguments =
      owner_component ||
      component_matches_owner_template ||
      (options.owner_template_name.empty() && is_last_qualifier);

  if(structured_arguments &&
     owner_template_arguments_can_use_structured_mangling(*structured_arguments) &&
     should_use_structured_owner_arguments &&
     has_parsed_component &&
     component.has_template_id &&
     !component.base_name.empty()) {
    vector<abi_mangle::TemplateArgument> arguments;
    const bool structured_arguments_ok =
        structured_arguments->empty() ||
        try_build_function_template_arguments_ir(*structured_arguments,
                                                 structured_parameters,
                                                 nullptr,
                                                 &owner_component_ctx,
                                                 arguments);
    if(structured_arguments_ok) {
      const string base_name = trim_space(component.base_name);
      const string full_base_text =
          append_qualified_component_text(canonical_prefix,
                                          canonical_component_text(base_name));
      const string complete_text =
          append_qualified_component_text(canonical_prefix,
                                          canonical_component_text(part));
      string standard_substitution;
      bool standard_substitution_includes_arguments = false;
      structured_std_standard_substitution_for_template_component(
          base_name,
          *structured_arguments,
          canonical_prefix,
          &owner_component_ctx,
          standard_substitution,
          standard_substitution_includes_arguments);
      vector<abi_mangle::SubstitutionKey> argument_keys;
      const bool have_argument_keys =
          !standard_substitution_includes_arguments &&
          build_function_template_argument_ir_substitution_keys(arguments,
                                                               argument_keys);
      out = abi_mangle::FunctionNameComponent::template_component(
          base_name,
          full_base_text,
          complete_text,
          std::move(arguments),
          std::move(standard_substitution),
          standard_substitution_includes_arguments);
      if(have_argument_keys) {
        out.complete_ir_substitution_key =
            abi_mangle::SubstitutionKey::
                class_template_specialization(0,
                                              full_base_text,
                                              std::move(argument_keys));
      }
      canonical_prefix = complete_text;
      owner_template_component_index = candidate_owner_template_component_index;
      return true;
    }
    if(owner_component) {
      return false;
    }
  }

  if(has_parsed_component &&
     component.has_template_id &&
     !component.base_name.empty()) {
    TemplateIdSyntax template_id;
    const string structured_template_name = owner_component ?
        owner_component->template_name :
        (component_matches_lexical_owner_template ?
             lexical_owner->source_template->name :
             (component_matches_owner_template ? options.owner_template_name : string()));
    if(!structured_template_name.empty() && structured_arguments) {
      template_id.name.name = structured_template_name;
      template_id.arguments.reserve(structured_arguments->size());
      template_id.argument_syntaxes.reserve(structured_arguments->size());
      for(size_t i = 0; i < structured_arguments->size(); ++i) {
        const TemplateArgument & argument = (*structured_arguments)[i];
        TemplateArgumentSyntax syntax = argument.source_syntax ?
            clone_template_argument_syntax_for_mangling(*argument.source_syntax) :
            TemplateArgumentSyntax();
        if(syntax.text.empty()) {
          syntax.text = !argument.text.empty() ? argument.text :
              (argument.kind == TemplateArgument::TA_TYPE && argument.type ?
                   template_argument_type_text(argument.type) :
                   argument.kind == TemplateArgument::TA_VALUE ?
                       to_string(argument.value) : string());
        }
        template_id.arguments.push_back(syntax.text);
        template_id.argument_syntaxes.push_back(std::move(syntax));
      }
      vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
      dependent_arguments.reserve(template_id.argument_syntaxes.size());
      for(size_t i = 0; i < template_id.argument_syntaxes.size(); ++i) {
        DependentAliasTemplateArgumentSyntax argument;
        argument.syntax = template_id.argument_syntaxes[i];
        argument.text = argument.syntax.text;
        if(argument.text.empty() && i < template_id.arguments.size()) {
          argument.text = template_id.arguments[i];
          argument.syntax.text = argument.text;
        }
        dependent_arguments.push_back(std::move(argument));
      }

      const vector<TemplateParameterInfo> * raw_parameters =
          structured_parameters ? structured_parameters :
          lookup_template_parameters_for_template_id_syntax(template_id,
                                                            &owner_component_ctx);
      vector<TemplateParameterInfo> inferred_parameters;
      if(!raw_parameters) {
        bool can_infer_parameters = !template_id.argument_syntaxes.empty();
        inferred_parameters.reserve(template_id.argument_syntaxes.size());
        for(size_t i = 0; can_infer_parameters &&
                        i < template_id.argument_syntaxes.size(); ++i) {
          const TemplateArgumentSyntax & syntax = template_id.argument_syntaxes[i];
          if(syntax.type_id || syntax.template_id || syntax.expression) {
            can_infer_parameters = false;
            break;
          }
          string parameter_name = trim_elaborated_type_prefix(syntax.text);
          bool parameter_pack = syntax.pack_expansion;
          if(parameter_name.size() >= 3 &&
             parameter_name.compare(parameter_name.size() - 3, 3, "...") == 0) {
            parameter_pack = true;
            parameter_name =
                trim_space(parameter_name.substr(0, parameter_name.size() - 3));
          }
          if(parameter_name.empty() ||
             !is_identifier_text_for_mangling(parameter_name)) {
            can_infer_parameters = false;
            break;
          }
          TemplateParameterInfo parameter;
          parameter.kind = TemplateParameterInfo::TP_TYPE;
          parameter.name = parameter_name;
          parameter.parameter_pack = parameter_pack;
          inferred_parameters.push_back(parameter);
        }
        if(can_infer_parameters) {
          raw_parameters = &inferred_parameters;
        }
      }
      if(parser_trace::enabled("symbol.linkage")) {
        ostringstream trace;
        trace << "function-template-component-syntax"
              << " part=" << part
              << " raw-params=" << (raw_parameters ? raw_parameters->size() : 0)
              << " inferred=" << inferred_parameters.size()
              << " args=" << dependent_arguments.size()
              << " has-template-ctx="
              << (owner_component_ctx.template_parameters ? "yes" : "no")
              << " has-lookup=" << (owner_component_ctx.lookup_scope ? "yes" : "no");
        parser_trace::note("symbol.linkage", string(), trace.str());
      }
      if(raw_parameters &&
         dependent_arguments.size() < raw_parameters->size()) {
        dependent_arguments =
            complete_dependent_alias_template_arguments_for_mangling(
                *raw_parameters,
                dependent_arguments,
                template_id_default_argument_scope_for_mangling(
                    template_id,
                    &owner_component_ctx));
      }
      vector<TemplateParameterInfo> explicit_parameter_storage;
      const vector<TemplateParameterInfo> * parameters =
          template_id_parameters_for_ir(
              template_id,
              raw_parameters,
              options.suppress_template_argument_pack_grouping,
              explicit_parameter_storage);
      TypeMangleContext syntax_ctx = owner_component_ctx;
      TemplateParameterMangleContext syntax_template_ctx;
      if(parameters && !syntax_ctx.template_parameters) {
        syntax_template_ctx.parameters = parameters;
        syntax_ctx.template_parameters = &syntax_template_ctx;
      }

      vector<abi_mangle::Type::ClassTemplateArgument> class_arguments;
      if(!build_dependent_template_arguments_ir(dependent_arguments,
                                                parameters,
                                                &syntax_ctx,
                                                class_arguments)) {
        if(parser_trace::enabled("symbol.linkage")) {
          parser_trace::note("symbol.linkage",
                             string(),
                             "function-template-component-syntax arguments-failed");
        }
        return false;
      }
      vector<abi_mangle::TemplateArgument> arguments;
      arguments.reserve(class_arguments.size());
      for(size_t i = 0; i < class_arguments.size(); ++i) {
        abi_mangle::TemplateArgument argument;
        if(!class_template_argument_ir_to_template_argument_ir(
               class_arguments[i],
               argument)) {
          return false;
        }
        arguments.push_back(std::move(argument));
      }

      const string base_name = trim_space(component.base_name);
      const string full_base_text =
          append_qualified_component_text(canonical_prefix,
                                          canonical_component_text(base_name));
      const string complete_text =
          append_qualified_component_text(canonical_prefix,
                                          canonical_component_text(part));
      vector<TemplateArgument> source_arguments =
          template_arguments_for_dependent_mangling(dependent_arguments,
                                                    parameters,
                                                    &syntax_ctx);
      string standard_substitution;
      bool standard_substitution_includes_arguments = false;
      structured_std_standard_substitution_for_template_component(
          base_name,
          source_arguments,
          canonical_prefix,
          &syntax_ctx,
          standard_substitution,
          standard_substitution_includes_arguments);
      vector<abi_mangle::SubstitutionKey> argument_keys;
      const bool have_argument_keys =
          !standard_substitution_includes_arguments &&
          build_function_template_argument_ir_substitution_keys(arguments,
                                                               argument_keys);
      out = abi_mangle::FunctionNameComponent::template_component(
          base_name,
          full_base_text,
          complete_text,
          std::move(arguments),
          std::move(standard_substitution),
          standard_substitution_includes_arguments);
      if(have_argument_keys) {
        out.complete_ir_substitution_key =
            abi_mangle::SubstitutionKey::
                class_template_specialization(0,
                                              full_base_text,
                                              std::move(argument_keys));
      }
      canonical_prefix = complete_text;
      return true;
    }
  }

  if(!has_parsed_component ||
     component.has_template_id ||
     !is_identifier_text_for_mangling(part)) {
    return false;
  }
  const string canonical_component = canonical_component_text(part);
  if(canonical_prefix.empty() && canonical_component == "std") {
    out = abi_mangle::FunctionNameComponent::std_namespace();
    canonical_prefix = "std";
    return true;
  }
  const string full_name =
      append_qualified_component_text(canonical_prefix, canonical_component);
  out = abi_mangle::FunctionNameComponent::source(part, full_name);
  canonical_prefix = full_name;
  return true;
}

static bool build_function_name_components_ir(
    const QualifiedName & qualified,
    const FunctionSymbolOptions & options,
    bool include_terminal_source_name,
    abi_mangle::FunctionEncoding & function)
{
  if(qualified.rooted ||
     qualified.name.empty() ||
     (include_terminal_source_name &&
      (contains_template_suffix(qualified.name) ||
       !is_identifier_text_for_mangling(qualified.name)))) {
    return false;
  }
  function.name_components.clear();
  function.name_components.reserve(qualified.qualifiers.size() + 1);
  string canonical_prefix;
  size_t owner_template_component_index = 0;
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    abi_mangle::FunctionNameComponent component;
    if(!try_build_function_qualifier_component_ir(
           qualified.qualifiers[i],
           options,
           i + 1 == qualified.qualifiers.size(),
           canonical_prefix,
           owner_template_component_index,
           component)) {
      return false;
    }
    if(!component.standard_substitution.empty() &&
       !function.name_components.empty() &&
       function.name_components.back().std_abbrev) {
      function.name_components.pop_back();
    }
    function.name_components.push_back(std::move(component));
  }
  function.name_components.push_back(include_terminal_source_name ?
      abi_mangle::FunctionNameComponent::source(qualified.name, string()) :
      abi_mangle::FunctionNameComponent::source(string(), string()));
  return true;
}

static bool try_emit_qualified_name_encoding_ir(const QualifiedName & qualified,
                                                string & out)
{
  FunctionSymbolOptions options;
  abi_mangle::FunctionEncoding function;
  if(!build_function_name_components_ir(qualified, options, true, function)) {
    return false;
  }
  MangleSubstitutionState state;
  MangleIrSubstitutionSink sink(&state);
  string candidate;
  if(!abi_mangle::emit_function_name(function, candidate, &sink)) {
    return false;
  }
  out.swap(candidate);
  return true;
}

static bool try_emit_qualified_name_object_symbol_ir(const QualifiedName & qualified,
                                                     string & out)
{
  FunctionSymbolOptions options;
  abi_mangle::FunctionEncoding function;
  if(!build_function_name_components_ir(qualified, options, true, function)) {
    return false;
  }
  MangleSubstitutionState state;
  if(!emit_function_name_ir(function, &state, out)) {
    return false;
  }
  return true;
}

static bool build_function_template_arguments_for_name_ir(
    const QualifiedName & qualified,
    const FunctionSymbolOptions & options,
    abi_mangle::FunctionEncoding & function)
{
  if(!options.template_arguments || options.template_arguments->empty()) {
    return true;
  }

  TemplateParameterMangleContext template_argument_parameter_ctx;
  TypeMangleContext template_argument_mangle_ctx;
  initialize_function_template_argument_mangle_context(
      qualified,
      options,
      template_argument_parameter_ctx,
      template_argument_mangle_ctx);

  const vector<TemplateParameterInfo> * effective_parameters =
      options.suppress_template_argument_pack_grouping ?
          nullptr :
          options.template_parameters;
  if(!try_build_function_template_arguments_ir(
         *options.template_arguments,
         effective_parameters,
         template_argument_mangle_ctx.template_argument_pack_sizes,
         &template_argument_mangle_ctx,
         function.template_arguments)) {
    return false;
  }

  const string key = function_template_prefix_substitution_key(
      qualified.qualifiers,
      qualified.name);
  const string key_prefix = "function-template-prefix:";
  if(key.compare(0, key_prefix.size(), key_prefix) == 0) {
    function.template_prefix_key =
        abi_mangle::SubstitutionKey::function_template_prefix(
            key.substr(key_prefix.size()));
  }
  return true;
}

static bool try_mangle_function_name_prefix_ir(
    const QualifiedName & qualified,
    const string & display_name,
    const FunctionSymbolOptions & options,
    string & out,
    MangleSubstitutionState * state,
    abi_mangle::FunctionEncoding * captured_function = nullptr)
{
  if((!display_name.empty() && display_name != qualified.name) ||
     (!display_name.empty() && display_name[0] == '~') ||
     !function_options_are_simple_name_ir_candidate(options) ||
     function_name_requires_operator_ir_path(qualified,
                                            display_name,
                                            TypePtr(),
                                            options)) {
    return false;
  }

  abi_mangle::FunctionEncoding function;
  if(!build_function_name_components_ir(qualified, options, true, function)) {
    return false;
  }
  function.abi_tags = options.abi_tags;
  if(!build_function_template_arguments_for_name_ir(qualified,
                                                    options,
                                                    function)) {
    return false;
  }

  string candidate;
  if(!emit_function_name_ir(function, state, candidate)) {
    return false;
  }
  if(captured_function) {
    *captured_function = function;
  }
  out.swap(candidate);
  return true;
}

static bool try_mangle_member_function_name_prefix_ir(
    const QualifiedName & qualified,
    const string & display_name,
    const FunctionSymbolOptions & options,
    string & out,
    MangleSubstitutionState * state,
    abi_mangle::FunctionEncoding * captured_function = nullptr)
{
  if((!display_name.empty() && display_name != qualified.name) ||
     (!display_name.empty() && display_name[0] == '~') ||
     !function_options_are_simple_member_name_ir_candidate(options) ||
     qualified.qualifiers.empty() ||
     function_name_requires_operator_ir_path(qualified,
                                            display_name,
                                            TypePtr(),
                                            options)) {
    return false;
  }

  abi_mangle::FunctionEncoding function;
  if(!build_function_name_components_ir(qualified, options, true, function)) {
    return false;
  }
  function.abi_tags = options.abi_tags;
  apply_member_function_nested_qualifiers_ir(options, function);
  if(!build_function_template_arguments_for_name_ir(qualified,
                                                    options,
                                                    function)) {
    return false;
  }

  string candidate;
  if(!emit_function_name_ir(function, state, candidate)) {
    return false;
  }
  if(captured_function) {
    *captured_function = function;
  }
  out.swap(candidate);
  return true;
}

static bool build_lambda_call_operator_template_arguments_ir(
    const QualifiedName & qualified,
    const FunctionSymbolOptions & options,
    abi_mangle::FunctionEncoding & function)
{
  if(!options.template_arguments || options.template_arguments->empty()) {
    return true;
  }

  TemplateParameterMangleContext template_argument_parameter_ctx;
  TypeMangleContext template_argument_mangle_ctx;
  initialize_function_template_argument_mangle_context(
      qualified,
      options,
      template_argument_parameter_ctx,
      template_argument_mangle_ctx);

  const vector<TemplateParameterInfo> * effective_parameters =
      options.suppress_template_argument_pack_grouping ?
          nullptr :
          options.template_parameters;
  return try_build_function_template_arguments_ir(
      *options.template_arguments,
      effective_parameters,
      template_argument_mangle_ctx.template_argument_pack_sizes,
      &template_argument_mangle_ctx,
      function.template_arguments);
}

static bool try_mangle_lambda_call_operator_name_prefix_ir(
    const QualifiedName & qualified,
    const string & display_name,
    const FunctionSymbolOptions & options,
    string & out,
    MangleSubstitutionState * state,
    abi_mangle::FunctionEncoding * captured_function = nullptr)
{
  if(qualified.rooted ||
     display_name != "operator()" ||
     !options.is_member_function ||
     !options.lambda_closure_type) {
    return false;
  }

  abi_mangle::FunctionEncoding function;
  function.abi_tags = options.abi_tags;
  if(!initialize_local_entity_function_metadata(options.lambda_closure_type,
                                                nullptr,
                                                function)) {
    return false;
  }
  apply_member_function_nested_qualifiers_ir(options, function);
  if(!build_lambda_call_operator_template_arguments_ir(qualified,
                                                       options,
                                                       function)) {
    return false;
  }

  string candidate;
  if(!emit_function_name_ir(function, state, candidate)) {
    return false;
  }
  if(captured_function) {
    *captured_function = function;
  }
  out.swap(candidate);
  return true;
}

static bool try_mangle_special_member_name_prefix_ir(
    const QualifiedName & qualified,
    const FunctionSymbolOptions & options,
    string & out,
    MangleSubstitutionState * state,
    abi_mangle::FunctionEncoding * captured_function = nullptr)
{
  if(qualified.rooted ||
     qualified.qualifiers.empty() ||
     (!options.is_constructor && !options.is_destructor) ||
     options.is_member_function == false ||
     (options.is_destructor &&
      options.template_arguments &&
      !options.template_arguments->empty())) {
    return false;
  }

  string special_code;
  if(options.is_constructor) {
    special_code =
        options.special_member_entry_point_kind == SMEK_BASE ? "C2" : "C1";
  } else {
    special_code =
        options.special_member_entry_point_kind == SMEK_BASE ? "D2" :
        options.special_member_entry_point_kind == SMEK_DELETING ? "D0" :
                                                                  "D1";
  }

  abi_mangle::FunctionEncoding function;
  function.abi_tags = options.abi_tags;
  function.terminal_fragment = special_code;
  TypePtr local_special_member_type =
      options.lambda_closure_type ? options.lambda_closure_type :
                                    options.local_class_type;
  if(local_special_member_type) {
    if(!initialize_local_entity_function_metadata(local_special_member_type,
                                                  nullptr,
                                                  function)) {
      return false;
    }
  } else if(!build_function_name_components_ir(qualified, options, false, function)) {
    if(parser_trace::enabled("symbol.linkage")) {
      parser_trace::note("symbol.linkage",
                         string(),
                         "special-member-ir components-failed");
    }
    return false;
  }
  if(options.is_constructor &&
     options.template_arguments &&
     !options.template_arguments->empty() &&
     !build_function_template_arguments_for_name_ir(qualified,
                                                    options,
                                                    function)) {
    if(parser_trace::enabled("symbol.linkage")) {
      parser_trace::note("symbol.linkage",
                         string(),
                         "special-member-ir template-arguments-failed");
    }
    return false;
  }

  string candidate;
  if(!emit_function_name_ir(function, state, candidate)) {
    if(parser_trace::enabled("symbol.linkage")) {
      ostringstream trace;
      trace << "special-member-ir emit-failed candidate=" << candidate
            << " name-components=" << function.name_components.size()
            << " template-args=" << function.template_arguments.size();
      parser_trace::note("symbol.linkage", string(), trace.str());
    }
    return false;
  }
  if(captured_function) {
    *captured_function = function;
  }
  out.swap(candidate);
  return true;
}

static bool try_mangle_local_class_member_name_prefix_ir(
    const QualifiedName & qualified,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    string & out,
    MangleSubstitutionState * state,
    abi_mangle::FunctionEncoding * captured_function = nullptr)
{
  if(qualified.rooted ||
     qualified.qualifiers.empty() ||
     !options.local_class_type ||
     !options.is_member_function ||
     options.is_constructor ||
     options.is_destructor) {
    return false;
  }

  const string & display_or_name =
      display_name.empty() ? qualified.name : display_name;
  if(display_or_name.empty() || display_or_name[0] == '~') {
    return false;
  }

  abi_mangle::FunctionEncoding function;
  function.abi_tags = options.abi_tags;
  if(!initialize_local_entity_function_metadata(options.local_class_type,
                                                nullptr,
                                                function)) {
    return false;
  }
  apply_member_function_nested_qualifiers_ir(options, function);

  if(function_name_requires_operator_ir_path(qualified,
                                            display_name,
                                            type,
                                            options)) {
    const string compact = compact_operator_name(display_or_name);
    abi_mangle::FunctionOperatorTerminal operator_terminal =
        abi_mangle::FUNCTION_OPERATOR_NONE;
    string operator_literal_suffix;
    if(fixed_operator_mangle_terminal(compact,
                                      operator_explicit_parameter_count(
                                          qualified,
                                          type,
                                          options),
                                      true,
                                      operator_terminal,
                                      operator_literal_suffix)) {
      function.operator_terminal = operator_terminal;
      function.operator_literal_suffix = operator_literal_suffix;
    } else {
      TypeMangleContext mangle_ctx;
      mangle_ctx.lexical_scope = function_type_lexical_scope(qualified,
                                                             options);
      mangle_ctx.lexical_scope_syntax =
          function_type_lexical_scope_syntax(qualified, options);
      mangle_ctx.lookup_scope = options.lookup_scope;
      mangle_ctx.owner_template_parameters = options.owner_template_parameters;
      mangle_ctx.owner_template_arguments = options.owner_template_arguments;

      TypePtr target_type;
      TemplateParameterMangleContext template_ctx;
      if(options.template_parameters &&
         options.template_arguments &&
         options.parameter_pattern &&
         !options.template_arguments->empty()) {
        template_ctx.parameters = options.template_parameters;
        TypePtr pattern_type = strip_top_level_cv(options.function_type_pattern);
        if(pattern_type && pattern_type->kind == Type::TK_FUNCTION) {
          target_type = pattern_type->inner;
          mangle_ctx.template_parameters = &template_ctx;
          mangle_ctx.allow_direct_std_standard_substitutions = false;
        }
      }
      TypePtr function_type = strip_top_level_cv(type);
      if(!target_type &&
         (!function_type || function_type->kind != Type::TK_FUNCTION)) {
        return false;
      }
      if(!target_type) {
        target_type = function_type->inner;
      }
      abi_mangle::Type conversion_type;
      if(!target_type ||
         !try_build_type_ir(target_type, &mangle_ctx, conversion_type)) {
        return false;
      }
      function.conversion_type.reset(new abi_mangle::Type(
          std::move(conversion_type)));
      function.has_conversion_type = true;
    }
  } else {
    if((!display_name.empty() && display_name != qualified.name) ||
       !is_identifier_text_for_mangling(qualified.name)) {
      return false;
    }
    function.terminal_source_name = qualified.name;
  }

  if(!build_lambda_call_operator_template_arguments_ir(qualified,
                                                       options,
                                                       function)) {
    return false;
  }

  string candidate;
  if(!emit_function_name_ir(function, state, candidate)) {
    return false;
  }
  if(captured_function) {
    *captured_function = function;
  }
  out.swap(candidate);
  return true;
}

static bool try_mangle_operator_name_prefix_ir(
    const QualifiedName & qualified,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    string & out,
    MangleSubstitutionState * state,
    abi_mangle::FunctionEncoding * captured_function = nullptr)
{
  const string compact =
      compact_operator_name(display_name.empty() ? qualified.name : display_name);
  if(qualified.rooted ||
     compact.compare(0, 8, "operator") != 0 ||
     !function_options_are_fixed_operator_name_ir_candidate(options) ||
     (options.is_member_function && qualified.qualifiers.empty())) {
    return false;
  }

  const size_t explicit_param_count =
      operator_explicit_parameter_count(qualified, type, options);
  abi_mangle::FunctionOperatorTerminal operator_terminal =
      abi_mangle::FUNCTION_OPERATOR_NONE;
  string operator_literal_suffix;
  if(!fixed_operator_mangle_terminal(compact,
                                     explicit_param_count,
                                     options.is_member_function,
                                     operator_terminal,
                                     operator_literal_suffix)) {
    return false;
  }

  abi_mangle::FunctionEncoding function;
  function.abi_tags = options.abi_tags;
  function.operator_terminal = operator_terminal;
  function.operator_literal_suffix = operator_literal_suffix;
  if(options.is_member_function) {
    apply_member_function_nested_qualifiers_ir(options, function);
  }
  if(qualified.qualifiers.empty()) {
    function.name_fragment = "operator";
  } else {
    if(!build_function_name_components_ir(qualified, options, false, function)) {
      return false;
    }
  }

  if(options.template_arguments && !options.template_arguments->empty()) {
    TemplateParameterMangleContext template_argument_parameter_ctx;
    TypeMangleContext template_argument_mangle_ctx;
    initialize_function_template_argument_mangle_context(
        qualified,
        options,
        template_argument_parameter_ctx,
        template_argument_mangle_ctx);
    const vector<TemplateParameterInfo> * effective_parameters =
        options.suppress_template_argument_pack_grouping ?
            nullptr :
            options.template_parameters;
    if(!try_build_function_template_arguments_ir(
           *options.template_arguments,
           effective_parameters,
           template_argument_mangle_ctx.template_argument_pack_sizes,
           &template_argument_mangle_ctx,
           function.template_arguments)) {
      return false;
    }
    string operator_key;
    if(!abi_mangle::function_operator_terminal_substitution_text(
           function.operator_terminal,
           function.operator_literal_suffix,
           operator_key)) {
      return false;
    }
    function.template_prefix_key =
        abi_mangle::SubstitutionKey::function_template_prefix(operator_key);
  }

  string candidate;
  if(!emit_function_name_ir(function, state, candidate)) {
    return false;
  }
  if(captured_function) {
    *captured_function = function;
  }
  out.swap(candidate);
  return true;
}

static bool try_mangle_conversion_operator_name_prefix_ir(
    const QualifiedName & qualified,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    string & out,
    MangleSubstitutionState * state,
    abi_mangle::FunctionEncoding * captured_function = nullptr)
{
  const string compact =
      compact_operator_name(display_name.empty() ? qualified.name : display_name);
  abi_mangle::FunctionOperatorTerminal fixed_terminal =
      abi_mangle::FUNCTION_OPERATOR_NONE;
  string fixed_literal_suffix;
  if(qualified.rooted ||
     compact.compare(0, 8, "operator") != 0 ||
     fixed_operator_mangle_terminal(compact,
                                    operator_explicit_parameter_count(
                                        qualified,
                                        type,
                                        options),
                                    options.is_member_function,
                                    fixed_terminal,
                                    fixed_literal_suffix) ||
     !options.is_conversion_operator ||
     !function_options_are_fixed_operator_name_ir_candidate(options) ||
     !options.is_member_function ||
     qualified.qualifiers.empty()) {
    return false;
  }

  TypeMangleContext mangle_ctx;
  mangle_ctx.lexical_scope = function_type_lexical_scope(qualified, options);
  mangle_ctx.lexical_scope_syntax =
      function_type_lexical_scope_syntax(qualified, options);
  mangle_ctx.lookup_scope = options.lookup_scope;
  mangle_ctx.owner_template_parameters = options.owner_template_parameters;
  mangle_ctx.owner_template_arguments = options.owner_template_arguments;

  TypePtr target_type;
  TemplateParameterMangleContext template_ctx;
  if(options.template_parameters &&
     options.template_arguments &&
     options.parameter_pattern &&
     !options.template_arguments->empty()) {
    template_ctx.parameters = options.template_parameters;
    TypePtr pattern_type = strip_top_level_cv(options.function_type_pattern);
    if(pattern_type && pattern_type->kind == Type::TK_FUNCTION) {
      target_type = pattern_type->inner;
      mangle_ctx.template_parameters = &template_ctx;
      mangle_ctx.allow_direct_std_standard_substitutions = false;
    }
  }
  TypePtr function_type = strip_top_level_cv(type);
  if(!target_type &&
     (!function_type || function_type->kind != Type::TK_FUNCTION)) {
    return false;
  }
  if(!target_type) {
    target_type = function_type->inner;
  }

  abi_mangle::Type conversion_type;
  bool built_conversion_type =
      try_build_type_ir(target_type, &mangle_ctx, conversion_type);
  if(!built_conversion_type) {
    if(parser_trace::enabled("symbol.linkage")) {
      ostringstream trace;
      trace << "conversion-ir target-type-failed"
            << " kind="
            << (target_type ? to_string(static_cast<int>(target_type->kind)) :
                string("<none>"))
            << " display="
            << (target_type ? target_type->named_display : string("<none>"))
            << " key="
            << (target_type ? target_type->named_key : string("<none>"));
      parser_trace::note("symbol.linkage", string(), trace.str());
    }
    return false;
  }

  abi_mangle::FunctionEncoding function;
  function.abi_tags = options.abi_tags;
  function.has_conversion_type = true;
  function.conversion_type.reset(new abi_mangle::Type(
      std::move(conversion_type)));
  apply_member_function_nested_qualifiers_ir(options, function);
  if(!build_function_name_components_ir(qualified, options, false, function)) {
    if(parser_trace::enabled("symbol.linkage")) {
      parser_trace::note("symbol.linkage",
                         string(),
                         "conversion-ir components-failed");
    }
    return false;
  }
  if(options.template_arguments &&
     !options.template_arguments->empty() &&
     !build_function_template_arguments_for_name_ir(qualified,
                                                    options,
                                                    function)) {
    return false;
  }

  string candidate;
  if(!emit_function_name_ir(function, state, candidate)) {
    return false;
  }
  if(captured_function) {
    *captured_function = function;
  }
  out.swap(candidate);
  return true;
}

static bool try_mangle_plain_function_ir(const QualifiedName & qualified,
                                         const string & display_name,
                                         const TypePtr & type,
                                         const FunctionSymbolOptions & options,
                                         string & out,
                                         MangleSubstitutionState * state,
                                         bool suppress_type_substitution_keys,
                                         abi_mangle::FunctionEncoding * captured_function = nullptr)
{
  if(qualified.rooted ||
     qualified.name.empty() ||
     contains_template_suffix(qualified.name) ||
     !is_identifier_text_for_mangling(qualified.name) ||
     (!display_name.empty() && display_name != qualified.name) ||
     !function_options_are_plain_ir_candidate(options) ||
     function_name_requires_operator_ir_path(qualified,
                                            display_name,
                                            type,
                                            options)) {
    return false;
  }
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    if(!is_identifier_text_for_mangling(qualified.qualifiers[i])) {
      return false;
    }
  }
  TypePtr function_type = strip_top_level_cv(type);
  if(!function_type ||
     function_type->kind != Type::TK_FUNCTION ||
     function_type->function_const ||
     function_type->function_volatile ||
     function_type->function_ref_qualifier != FTRQ_NONE) {
    return false;
  }

  abi_mangle::FunctionEncoding function;
  function.abi_tags = options.abi_tags;
  if(!build_function_name_components_ir(qualified, options, true, function)) {
    return false;
  }
  function.variadic = function_type->variadic;
  function.parameter_types.reserve(function_type->params.size());
  TypeMangleContext ir_only_type_ctx;
  ir_only_type_ctx.suppress_type_substitution_keys =
      suppress_type_substitution_keys;
  const TypeMangleContext * parameter_type_ctx =
      suppress_type_substitution_keys ? &ir_only_type_ctx : nullptr;
  for(size_t i = 0; i < function_type->params.size(); ++i) {
    abi_mangle::Type param;
    if(!try_build_type_ir_cached(function_type->params[i],
                                 parameter_type_ctx,
                                 param)) {
      return false;
    }
    function.parameter_types.push_back(std::move(param));
  }

  string candidate;
  if(captured_function) {
    if(!emit_function_encoding_ir(function, state, candidate)) {
      return false;
    }
    *captured_function = function;
  } else if(!emit_function_encoding_ir_owned(function, state, candidate)) {
    return false;
  }
  out.swap(candidate);
  return true;
}

enum FunctionNameIrPath
{
  FNIP_NONE,
  FNIP_SIMPLE_FREE,
  FNIP_SIMPLE_MEMBER,
  FNIP_LAMBDA_CALL,
  FNIP_SPECIAL_MEMBER,
  FNIP_LOCAL_CLASS_MEMBER,
  FNIP_FIXED_OPERATOR,
  FNIP_CONVERSION_OPERATOR
};

static FunctionNameIrPath select_function_name_ir_path(
    const QualifiedName & qualified,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options)
{
  if(qualified.rooted || qualified.name.empty()) {
    return FNIP_NONE;
  }

  const string & display_or_name =
      display_name.empty() ? qualified.name : display_name;

  if(options.lambda_closure_type) {
    if(display_name == "operator()" && options.is_member_function) {
      return FNIP_LAMBDA_CALL;
    }
    if((options.is_constructor || options.is_destructor) &&
       options.is_member_function &&
       !qualified.qualifiers.empty()) {
      return FNIP_SPECIAL_MEMBER;
    }
    return FNIP_NONE;
  }
  if(options.local_class_type) {
    if((options.is_constructor || options.is_destructor) &&
       options.is_member_function &&
       !qualified.qualifiers.empty()) {
      return FNIP_SPECIAL_MEMBER;
    }
    if(options.is_member_function && !qualified.qualifiers.empty()) {
      return FNIP_LOCAL_CLASS_MEMBER;
    }
    return FNIP_NONE;
  }

  if(options.is_constructor || options.is_destructor) {
    if(!qualified.qualifiers.empty() &&
       options.is_member_function &&
       (!options.is_destructor ||
        !options.template_arguments ||
        options.template_arguments->empty())) {
      return FNIP_SPECIAL_MEMBER;
    }
    return FNIP_NONE;
  }

  string compact;
  bool is_operator = false;
  if(name_starts_with_operator_text(display_or_name)) {
    compact = compact_operator_name(display_or_name);
    is_operator = compact.compare(0, 8, "operator") == 0;
  }

  if(is_operator) {
    if(function_options_are_fixed_operator_name_ir_candidate(options)) {
      abi_mangle::FunctionOperatorTerminal fixed_terminal =
          abi_mangle::FUNCTION_OPERATOR_NONE;
      string fixed_literal_suffix;
      const bool fixed_operator =
          fixed_operator_mangle_terminal(
              compact,
              operator_explicit_parameter_count(qualified, type, options),
              options.is_member_function,
              fixed_terminal,
              fixed_literal_suffix);
      if(fixed_operator) {
        return options.is_member_function && qualified.qualifiers.empty() ?
            FNIP_NONE :
            FNIP_FIXED_OPERATOR;
      }
      if(options.is_member_function &&
         !qualified.qualifiers.empty() &&
         options.is_conversion_operator) {
        return FNIP_CONVERSION_OPERATOR;
      }
    }
    if(!ordinary_identifier_operator_prefix_name(qualified, display_name)) {
      return FNIP_NONE;
    }
  }

  if((!display_name.empty() && display_name != qualified.name) ||
     (!display_name.empty() && display_name[0] == '~')) {
    return FNIP_NONE;
  }

  if(function_options_are_simple_name_ir_candidate(options)) {
    return FNIP_SIMPLE_FREE;
  }
  if(function_options_are_simple_member_name_ir_candidate(options) &&
     !qualified.qualifiers.empty()) {
    return FNIP_SIMPLE_MEMBER;
  }
  return FNIP_NONE;
}

static bool build_itanium_function_context_name_ir(
    const QualifiedName & qualified,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    FunctionNameIrPath selected_path,
    abi_mangle::FunctionEncoding & function)
{
  switch(selected_path) {
  case FNIP_SIMPLE_FREE:
    function.abi_tags = options.abi_tags;
    return build_function_name_components_ir(qualified, options, true, function) &&
           build_function_template_arguments_for_name_ir(qualified,
                                                         options,
                                                         function);

  case FNIP_SIMPLE_MEMBER:
    function.abi_tags = options.abi_tags;
    apply_member_function_nested_qualifiers_ir(options, function);
    return build_function_name_components_ir(qualified, options, true, function) &&
           build_function_template_arguments_for_name_ir(qualified,
                                                         options,
                                                         function);

  case FNIP_LAMBDA_CALL:
    function.abi_tags = options.abi_tags;
    if(!initialize_local_entity_function_metadata(options.lambda_closure_type,
                                                  nullptr,
                                                  function)) {
      return false;
    }
    apply_member_function_nested_qualifiers_ir(options, function);
    return build_lambda_call_operator_template_arguments_ir(qualified,
                                                            options,
                                                            function);

  case FNIP_SPECIAL_MEMBER: {
    if(!options.is_constructor && !options.is_destructor) {
      return false;
    }
    function.abi_tags = options.abi_tags;
    function.terminal_fragment =
        options.is_constructor ?
            (options.special_member_entry_point_kind == SMEK_BASE ? "C2" : "C1") :
            (options.special_member_entry_point_kind == SMEK_BASE ? "D2" :
             options.special_member_entry_point_kind == SMEK_DELETING ? "D0" :
                                                                         "D1");
    TypePtr local_special_member_type =
        options.lambda_closure_type ? options.lambda_closure_type :
                                      options.local_class_type;
    if(local_special_member_type) {
      if(!initialize_local_entity_function_metadata(local_special_member_type,
                                                    nullptr,
                                                    function)) {
        return false;
      }
    } else if(!build_function_name_components_ir(qualified,
                                                 options,
                                                 false,
                                                 function)) {
      return false;
    }
    if(options.is_constructor &&
       options.template_arguments &&
       !options.template_arguments->empty() &&
       !build_function_template_arguments_for_name_ir(qualified,
                                                      options,
                                                      function)) {
      return false;
    }
    return true;
  }

  case FNIP_LOCAL_CLASS_MEMBER: {
    if(!options.local_class_type) {
      return false;
    }
    if(function_name_requires_operator_ir_path(qualified,
                                              display_name,
                                              type,
                                              options) ||
       (!display_name.empty() && display_name != qualified.name) ||
       !is_identifier_text_for_mangling(qualified.name)) {
      return false;
    }
    function.abi_tags = options.abi_tags;
    if(!initialize_local_entity_function_metadata(options.local_class_type,
                                                  nullptr,
                                                  function)) {
      return false;
    }
    apply_member_function_nested_qualifiers_ir(options, function);
    function.terminal_source_name = qualified.name;
    return build_lambda_call_operator_template_arguments_ir(qualified,
                                                            options,
                                                            function);
  }

  case FNIP_FIXED_OPERATOR:
  case FNIP_CONVERSION_OPERATOR:
  case FNIP_NONE:
    return false;
  }
  return false;
}

static bool build_itanium_context_function_type_mangle_context(
    const QualifiedName & qualified,
    const FunctionSymbolOptions & options,
    TypeMangleContext & mangle_ctx,
    TemplateParameterMangleContext & template_ctx,
    TemplateParameterMangleContext & owner_mangle_template_ctx,
    vector<FunctionParameterMangleInfo> & function_parameter_context,
    bool & owner_template_only_pattern)
{
  mangle_ctx.lexical_scope = function_type_lexical_scope(qualified, options);
  mangle_ctx.lexical_scope_syntax =
      function_type_lexical_scope_syntax(qualified, options);
  mangle_ctx.lookup_scope = options.lookup_scope;
  mangle_ctx.owner_template_parameters = options.owner_template_parameters;
  mangle_ctx.owner_template_arguments = options.owner_template_arguments;
  owner_template_only_pattern =
      template_parameter_lists_match(options.template_parameters,
                                     options.owner_template_parameters);
  mangle_ctx.prefer_concrete_non_type_values_for_dependent_parameter_types =
      options.owner_template_arguments != nullptr ||
      options.template_parameters == nullptr ||
      owner_template_only_pattern;
  mangle_ctx.suppress_template_argument_pack_grouping =
      options.suppress_template_argument_pack_grouping;

  if(!options.template_parameters && options.owner_mangle_parameters) {
    owner_mangle_template_ctx.parameters = options.owner_mangle_parameters;
    mangle_ctx.template_parameters = &owner_mangle_template_ctx;
  }

  if(options.template_parameters &&
     options.template_arguments &&
     options.parameter_pattern &&
     !options.template_arguments->empty()) {
    template_ctx.parameters = options.template_parameters;
    mangle_ctx.template_parameters = &template_ctx;
    mangle_ctx.allow_direct_std_standard_substitutions = false;
    function_parameter_context =
        build_function_parameter_mangle_context(options);
    if(!function_parameter_context.empty()) {
      mangle_ctx.function_parameters = &function_parameter_context;
    }
  }
  return true;
}

static bool append_context_function_type_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    abi_mangle::FunctionEncoding & function)
{
  abi_mangle::Type ir_type;
  if(!try_build_type_ir(type, mangle_ctx, ir_type)) {
    return false;
  }
  function.parameter_types.push_back(std::move(ir_type));
  return true;
}

static bool append_context_function_parameter_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    bool pack_expansion,
    abi_mangle::FunctionEncoding & function)
{
  abi_mangle::Type ir_type;
  if(!try_build_type_ir(type, mangle_ctx, ir_type)) {
    return false;
  }
  if(pack_expansion) {
    ir_type = abi_mangle::Type::pack_expansion(std::move(ir_type));
    attach_context_free_type_ir_substitution(ir_type);
  }
  function.parameter_types.push_back(std::move(ir_type));
  return true;
}

static bool build_itanium_function_context_parameter_ir(
    const QualifiedName & qualified,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    abi_mangle::FunctionEncoding & function)
{
  TypePtr function_type = strip_top_level_cv(type);
  if(!function_type ||
     function_type->kind != Type::TK_FUNCTION ||
     function_type->function_const ||
     function_type->function_volatile) {
    return false;
  }

  TypeMangleContext mangle_ctx;
  TemplateParameterMangleContext template_ctx;
  TemplateParameterMangleContext owner_mangle_template_ctx;
  vector<FunctionParameterMangleInfo> function_parameter_context;
  bool owner_template_only_pattern = false;
  build_itanium_context_function_type_mangle_context(
      qualified,
      options,
      mangle_ctx,
      template_ctx,
      owner_mangle_template_ctx,
      function_parameter_context,
      owner_template_only_pattern);

  const bool strip_implicit_self =
      options.has_implicit_object_parameter && !function_type->params.empty();
  const size_t param_start = strip_implicit_self ? 1 : 0;
  function.parameter_types.clear();

  TypePtr pattern_type = strip_top_level_cv(options.function_type_pattern);
  if(options.template_parameters &&
     options.template_arguments &&
     options.parameter_pattern &&
     !options.template_arguments->empty()) {
    const bool has_function_pattern =
        pattern_type && pattern_type->kind == Type::TK_FUNCTION;
    if(!has_function_pattern && !options.is_constructor && !options.is_destructor) {
      return false;
    }
    if(!options.is_constructor && !options.is_destructor) {
      if(!pattern_type ||
         !pattern_type->inner ||
         !function_type->inner) {
        return false;
      }
      const TypePtr hybrid_return_type =
          hybridize_pattern_type_with_actual(pattern_type->inner,
                                             function_type->inner,
                                             &mangle_ctx);
      if(!append_context_function_type_ir(hybrid_return_type,
                                          &mangle_ctx,
                                          function)) {
        return false;
      }
    }

    if(options.parameter_pattern->empty()) {
      function.variadic = (pattern_type && pattern_type->variadic) ||
                          (!pattern_type && function_type->variadic);
      if(!function.variadic) {
        function.parameter_types.push_back(
            abi_mangle::Type::builtin("v"));
      }
      return true;
    }

    for(size_t i = 0; i < options.parameter_pattern->size(); ++i) {
      const size_t actual_index = i + param_start;
      TypePtr actual_param =
          actual_index < function_type->params.size() ?
              function_type->params[actual_index] :
              TypePtr();
      const TypePtr hybrid_param_type =
          hybridize_pattern_type_with_actual((*options.parameter_pattern)[i].second,
                                             actual_param,
                                             &mangle_ctx);
      const bool trailing_function_parameter_pack =
          options.has_trailing_function_parameter_pack &&
          i + 1 == options.parameter_pattern->size();
      const bool parameter_type_mentions_direct_template_parameter =
          type_mentions_function_template_parameter_slice(
              (*options.parameter_pattern)[i].second,
              &mangle_ctx);
      const bool emit_trailing_function_parameter_pack =
          trailing_function_parameter_pack &&
          parameter_type_mentions_direct_template_parameter;
      TypePtr parameter_type = hybrid_param_type;
      if(owner_template_only_pattern &&
         actual_param &&
         (!type_has_dependent_mangle_state(actual_param) ||
          type_has_concrete_template_id_spelling_for_mangling(actual_param,
                                                             &mangle_ctx))) {
        parameter_type = actual_param;
      }
      if(!append_context_function_parameter_ir(parameter_type,
                                              &mangle_ctx,
                                              emit_trailing_function_parameter_pack,
                                              function)) {
        return false;
      }
    }
    function.variadic = (pattern_type && pattern_type->variadic) ||
                        (!pattern_type && function_type->variadic);
    return true;
  }

  function.variadic = function_type->variadic;
  for(size_t i = param_start; i < function_type->params.size(); ++i) {
    if(!append_context_function_parameter_ir(function_type->params[i],
                                            &mangle_ctx,
                                            false,
                                            function)) {
      return false;
    }
  }
  return true;
}

static bool build_itanium_function_context_encoding_ir(
    const QualifiedName & qualified,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    abi_mangle::FunctionEncoding & out)
{
  const FunctionNameIrPath selected_path =
      select_function_name_ir_path(qualified, display_name, type, options);
  if(selected_path == FNIP_NONE) {
    return false;
  }

  abi_mangle::FunctionEncoding function;
  if(!build_itanium_function_context_name_ir(qualified,
                                             display_name,
                                             type,
                                             options,
                                             selected_path,
                                             function) ||
     !build_itanium_function_context_parameter_ir(qualified,
                                                 type,
                                                 options,
                                                 function)) {
    return false;
  }
  out = std::move(function);
  return true;
}

static bool try_emit_owner_type_parameter_pack_function_parameter_ir(
    const TypePtr & type,
    const TypeMangleContext * mangle_ctx,
    MangleSubstitutionState * state,
    string & out,
    abi_mangle::Type * captured_type = nullptr)
{
  if(!type_mentions_owner_type_template_parameter_pack(type, mangle_ctx)) {
    return false;
  }

  abi_mangle::Type pattern;
  if(!try_build_type_ir(type, mangle_ctx, pattern)) {
    return false;
  }
  abi_mangle::Type expansion =
      pattern.kind == abi_mangle::Type::TK_PACK_EXPANSION ?
          std::move(pattern) :
          abi_mangle::Type::pack_expansion(std::move(pattern));
  MangleIrSubstitutionSink sink(state);
  if(captured_type) {
    if(!abi_mangle::emit_type(expansion, out, &sink)) {
      return false;
    }
    *captured_type = expansion;
    return true;
  }
  if(!abi_mangle::emit_type_owned(expansion, out, &sink)) {
    return false;
  }
  return true;
}

static void capture_function_mangle_target(
    abi_mangle::AbiMangleTarget * captured_target,
    const string & qualified_name_label,
    const abi_mangle::FunctionEncoding & function)
{
  if(!captured_target) {
    return;
  }
  captured_target->kind = abi_mangle::ABI_MANGLE_FUNCTION;
  captured_target->qualified_name = qualified_name_label;
  captured_target->function = function;
}

static bool try_emit_itanium_function_symbol_ir(
    const QualifiedName & qualified,
    const string & qualified_name_label,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    string & out,
    MangleSubstitutionState * captured_state,
    bool complete_plain_parameter_substitutions,
    abi_mangle::AbiMangleTarget * captured_target)
{
  if(captured_target) {
    *captured_target = abi_mangle::AbiMangleTarget();
  }
  MangleSubstitutionState local_state;
  MangleSubstitutionState * state = captured_state ? captured_state : &local_state;
  if(!type || type->kind != Type::TK_FUNCTION || type->function_const ||
     type->function_volatile || type->function_ref_qualifier != FTRQ_NONE) {
    if(parser_trace::enabled("symbol.linkage")) {
      ostringstream trace;
      trace << "mangle-itanium-function invalid-type"
            << " qualified=" << qualified_name_label
            << " display=" << display_name
            << " has-type=" << (type ? "yes" : "no")
            << " kind=" << (type ? to_string(static_cast<int>(type->kind)) : string("<none>"))
            << " const=" << (type && type->function_const ? "yes" : "no")
            << " volatile=" << (type && type->function_volatile ? "yes" : "no")
            << " refq=" << (type ? static_cast<int>(type->function_ref_qualifier) : 0);
      parser_trace::note("symbol.linkage", string(), trace.str());
    }
    return false;
  }

  // Convergence: parameter types now carry correct whole-type substitution keys
  // (make_type_substitution_key builds named/class-template keys), so the typed
  // plain-IR path can produce complete, clang-correct parameter substitutions
  // (e.g. repeated std::string -> RS_/S4_). This was previously suppressed.
  const bool suppress_type_substitution_keys = false;
  (void)complete_plain_parameter_substitutions;
  const bool capture_function = captured_target != nullptr;
  abi_mangle::FunctionEncoding captured_function;
  if(try_mangle_plain_function_ir(qualified,
                                  display_name,
                                  type,
                                  options,
                                  out,
                                  state,
                                  suppress_type_substitution_keys,
                                  capture_function ? &captured_function : nullptr)) {
    capture_function_mangle_target(captured_target,
                                   qualified_name_label,
                                   captured_function);
    return true;
  }

  string candidate;
  bool candidate_ok = false;
  bool have_captured_function = false;
  const FunctionNameIrPath selected_path =
      select_function_name_ir_path(qualified, display_name, type, options);
  switch(selected_path) {
  case FNIP_SIMPLE_FREE:
    candidate_ok = try_mangle_function_name_prefix_ir(qualified,
                                                      display_name,
                                                      options,
                                                      candidate,
                                                      state,
                                                      capture_function ? &captured_function :
                                                                         nullptr);
    break;
  case FNIP_SIMPLE_MEMBER:
    candidate_ok = try_mangle_member_function_name_prefix_ir(qualified,
                                                             display_name,
                                                             options,
                                                             candidate,
                                                             state,
                                                             capture_function ? &captured_function :
                                                                                nullptr);
    break;
  case FNIP_LAMBDA_CALL:
    candidate_ok = try_mangle_lambda_call_operator_name_prefix_ir(qualified,
                                                                  display_name,
                                                                  options,
                                                                  candidate,
                                                                  state,
                                                                  capture_function ? &captured_function :
                                                                                     nullptr);
    break;
  case FNIP_SPECIAL_MEMBER:
    candidate_ok = try_mangle_special_member_name_prefix_ir(qualified,
                                                            options,
                                                            candidate,
                                                            state,
                                                            capture_function ? &captured_function :
                                                                               nullptr);
    break;
  case FNIP_LOCAL_CLASS_MEMBER:
    candidate_ok = try_mangle_local_class_member_name_prefix_ir(qualified,
                                                                display_name,
                                                                type,
                                                                options,
                                                                candidate,
                                                                state,
                                                                capture_function ? &captured_function :
                                                                                   nullptr);
    break;
  case FNIP_FIXED_OPERATOR:
    candidate_ok = try_mangle_operator_name_prefix_ir(qualified,
                                                      display_name,
                                                      type,
                                                      options,
                                                      candidate,
                                                      state,
                                                      capture_function ? &captured_function :
                                                                         nullptr);
    break;
  case FNIP_CONVERSION_OPERATOR:
    candidate_ok = try_mangle_conversion_operator_name_prefix_ir(qualified,
                                                                 display_name,
                                                                 type,
                                                                 options,
                                                                 candidate,
                                                                 state,
                                                                 capture_function ? &captured_function :
                                                                                    nullptr);
    break;
  case FNIP_NONE:
    candidate_ok = false;
    break;
  }

  if(!candidate_ok) {
    if(parser_trace::enabled("symbol.linkage")) {
      ostringstream trace;
      trace << "mangle-itanium-function qualified-name-failed"
            << " qualified=" << qualified_name_label
            << " display=" << display_name
            << " path=" << static_cast<int>(selected_path)
            << " ctor=" << (options.is_constructor ? "yes" : "no")
            << " dtor=" << (options.is_destructor ? "yes" : "no")
            << " member=" << (options.is_member_function ? "yes" : "no")
            << " lambda=" << (options.lambda_closure_type ? "yes" : "no")
            << " local=" << (options.local_class_type ? "yes" : "no")
            << " template-arg-count="
            << (options.template_arguments ? options.template_arguments->size() : 0)
            << " template-param-count="
            << (options.template_parameters ? options.template_parameters->size() : 0)
            << " owner-arg-count="
            << (options.owner_template_arguments ?
                    options.owner_template_arguments->size() : 0)
            << " owner-param-count="
            << (options.owner_template_parameters ?
                    options.owner_template_parameters->size() : 0)
            << " owner-components=" << options.owner_template_components.size();
      parser_trace::note("symbol.linkage", string(), trace.str());
    }
    return false;
  }
  have_captured_function = capture_function;

  TypeMangleContext mangle_ctx;
  mangle_ctx.lexical_scope = function_type_lexical_scope(qualified, options);
  mangle_ctx.lexical_scope_syntax =
      function_type_lexical_scope_syntax(qualified, options);
  mangle_ctx.lookup_scope = options.lookup_scope;
  mangle_ctx.owner_template_parameters = options.owner_template_parameters;
  mangle_ctx.owner_template_arguments = options.owner_template_arguments;
  const bool owner_template_only_pattern =
      template_parameter_lists_match(options.template_parameters,
                                     options.owner_template_parameters);
  mangle_ctx.prefer_concrete_non_type_values_for_dependent_parameter_types =
      options.owner_template_arguments != nullptr ||
      options.template_parameters == nullptr ||
      owner_template_only_pattern;
  mangle_ctx.suppress_template_argument_pack_grouping =
      options.suppress_template_argument_pack_grouping;
  const bool strip_implicit_self =
      options.has_implicit_object_parameter && !type->params.empty();
  const size_t param_start = strip_implicit_self ? 1 : 0;

  TemplateParameterMangleContext template_ctx;
  TemplateParameterMangleContext owner_mangle_template_ctx;
  if(!options.template_parameters && options.owner_mangle_parameters) {
    owner_mangle_template_ctx.parameters = options.owner_mangle_parameters;
    mangle_ctx.template_parameters = &owner_mangle_template_ctx;
  }
  TypePtr pattern_type = strip_top_level_cv(options.function_type_pattern);
  if(options.template_parameters &&
     options.template_arguments &&
     options.parameter_pattern &&
     !options.template_arguments->empty()) {
    const bool has_function_pattern =
        pattern_type && pattern_type->kind == Type::TK_FUNCTION;
    if(!has_function_pattern && !options.is_constructor && !options.is_destructor) {
      return false;
    }
    template_ctx.parameters = options.template_parameters;
    mangle_ctx.template_parameters = &template_ctx;
    mangle_ctx.allow_direct_std_standard_substitutions = false;
    const vector<FunctionParameterMangleInfo> function_parameter_context =
        build_function_parameter_mangle_context(options);
    if(!function_parameter_context.empty()) {
      mangle_ctx.function_parameters = &function_parameter_context;
    }
    TypePtr actual_function_type = strip_top_level_cv(type);
    if(!actual_function_type || actual_function_type->kind != Type::TK_FUNCTION) {
      return false;
    }
    if(!options.is_constructor && !options.is_destructor) {
      if(!actual_function_type->inner ||
         !pattern_type ||
         !pattern_type->inner) {
        return false;
      }
      bool mangled_return = false;
      bool have_captured_return_type = false;
      unique_ptr<abi_mangle::Type> captured_return_type;
      if(have_captured_function) {
        captured_return_type.reset(new abi_mangle::Type);
      }
      const bool has_result_type_pattern =
          options.result_type_pattern &&
          options.result_type_pattern->kind != CppAstKind::invalid &&
          !options.result_type_pattern->children.empty();
      const bool result_pattern_depends =
          (has_result_type_pattern &&
           (ast_node_mentions_direct_template_parameter(*options.result_type_pattern,
                                                       &mangle_ctx) ||
            ast_node_mentions_function_parameter(*options.result_type_pattern,
                                                &mangle_ctx))) ||
          text_mentions_direct_template_mangle_parameter(
              selected_named_type_text(pattern_type->inner), &mangle_ctx);
      EnableIfConditionDependency direct_enable_if_result_dependency;
      if(has_result_type_pattern) {
        collect_enable_if_condition_dependency(*options.result_type_pattern,
                                               &mangle_ctx,
                                               direct_enable_if_result_dependency);
      }
      EnableIfConditionDependency alias_enable_if_result_dependency;
      const bool result_references_alias_template =
          has_result_type_pattern &&
          type_ast_references_alias_template(*options.result_type_pattern,
                                             &mangle_ctx);
      if(result_references_alias_template) {
        alias_enable_if_result_dependency =
            result_alias_enable_if_condition_dependency(
                *options.result_type_pattern,
                &mangle_ctx);
      }
      const bool dependent_enable_if_result =
          (direct_enable_if_result_dependency.found &&
           direct_enable_if_result_dependency.dependent) ||
          (alias_enable_if_result_dependency.found &&
           alias_enable_if_result_dependency.dependent);
      const bool nondependent_enable_if_alias_result =
          alias_enable_if_result_dependency.found &&
          !alias_enable_if_result_dependency.dependent;
      TypePtr hybrid_return_type =
          hybridize_pattern_type_with_actual(pattern_type->inner,
                                             actual_function_type->inner,
                                             &mangle_ctx);
      const bool return_type_has_owner_dependent_mangle_state =
          type_has_structured_dependent_qualified_member(hybrid_return_type) ||
          type_has_dependent_class_template_nested_owner_mangle_state(
              hybrid_return_type);
      bool typed_return_substitutes_parameter = false;
      if(return_type_has_owner_dependent_mangle_state) {
        abi_mangle::SubstitutionKey return_key;
        if(type_ir_substitution_key_for_type(hybrid_return_type,
                                             &mangle_ctx,
                                             return_key)) {
          for(size_t i = 0; i < options.parameter_pattern->size(); ++i) {
            const size_t actual_index = i + param_start;
            TypePtr actual_param =
                actual_index < actual_function_type->params.size() ?
                    actual_function_type->params[actual_index] :
                    TypePtr();
            const TypePtr hybrid_param_type =
                hybridize_pattern_type_with_actual(
                    (*options.parameter_pattern)[i].second,
                    actual_param,
                    &mangle_ctx);
            abi_mangle::SubstitutionKey param_key;
            if(type_ir_substitution_key_for_type(hybrid_param_type,
                                                 &mangle_ctx,
                                                 param_key) &&
               param_key == return_key) {
              typed_return_substitutes_parameter = true;
              break;
            }
          }
        }
      }
      if(typed_return_substitutes_parameter) {
        const size_t structured_begin = candidate.size();
        mangled_return = build_and_emit_type_ir(hybrid_return_type,
                                                &mangle_ctx,
                                                state,
                                                candidate,
                                                captured_return_type.get());
        if(!mangled_return) {
          candidate.resize(structured_begin);
        } else {
          have_captured_return_type = true;
        }
      }
      if(!mangled_return &&
         has_result_type_pattern &&
         result_pattern_depends &&
         !nondependent_enable_if_alias_result) {
        const size_t structured_begin = candidate.size();
        TypeMangleContext result_ctx = mangle_ctx;
        if(dependent_enable_if_result) {
          result_ctx.canonical_enable_if_result_alias_substitutions = true;
        }
        if(ast_contains_decltype_specifier(*options.result_type_pattern) ||
           dependent_enable_if_result) {
          result_ctx.suppress_current_type_id_substitution_registration = true;
        }
        mangled_return = build_and_emit_type_id_ir_from_ast(
            *options.result_type_pattern,
            actual_function_type->inner,
            &result_ctx,
            state,
            candidate,
            captured_return_type.get());
        if(!mangled_return) {
          candidate.resize(structured_begin);
          if(actual_function_type->inner &&
             !type_has_dependent_mangle_state(actual_function_type->inner) &&
             build_and_emit_type_ir(actual_function_type->inner,
                                    &result_ctx,
                                    state,
                                    candidate,
                                    captured_return_type.get())) {
            mangled_return = true;
          } else if(type_ast_references_alias_template(*options.result_type_pattern,
                                                       &result_ctx) &&
                    build_and_emit_type_ir(actual_function_type->inner,
                                           &result_ctx,
                                           state,
                                           candidate,
                                           captured_return_type.get())) {
            mangled_return = true;
          }
          have_captured_return_type = mangled_return;
        } else {
          have_captured_return_type = true;
        }
        if(!mangled_return) {
          throw logic_error(
              string("dependent function result type reached unstructured ABI text mangler: ") +
              ast_node_diagnostic_label(*options.result_type_pattern));
        }
      }
      if(!mangled_return) {
        if(!build_and_emit_type_ir(hybrid_return_type,
                                   &mangle_ctx,
                                   state,
                                   candidate,
                                   captured_return_type.get())) {
          if(return_type_has_owner_dependent_mangle_state &&
             actual_function_type->inner &&
             !type_has_dependent_mangle_state(actual_function_type->inner) &&
             build_and_emit_type_ir(actual_function_type->inner,
                                    &mangle_ctx,
                                    state,
                                    candidate,
                                    captured_return_type.get())) {
            mangled_return = true;
            have_captured_return_type = true;
          } else {
            return false;
          }
        }
        if(!mangled_return) {
          have_captured_return_type = true;
        }
      }
	      if(have_captured_function && have_captured_return_type) {
	        captured_function.has_result_type = true;
        captured_function.result_type.reset(new abi_mangle::Type(
            std::move(*captured_return_type)));
	      }
    }
    if(options.parameter_pattern->empty()) {
      if(parser_trace::enabled("symbol.linkage")) {
        ostringstream trace;
        trace << "mangle-itanium-function template-param-tail-empty"
              << " qualified=" << qualified_name_label
              << " display=" << display_name
              << " pattern-param-count=" << options.parameter_pattern->size()
              << " actual-param-count=" << actual_function_type->params.size()
              << " ctor=" << (options.is_constructor ? "yes" : "no")
              << " dtor=" << (options.is_destructor ? "yes" : "no");
        parser_trace::note("symbol.linkage", string(), trace.str());
      }
      candidate += ((pattern_type && pattern_type->variadic) ||
                    (!pattern_type && actual_function_type->variadic)) ?
          'z' :
          'v';
      if(have_captured_function) {
        captured_function.variadic =
            (pattern_type && pattern_type->variadic) ||
            (!pattern_type && actual_function_type->variadic);
        captured_function.parameter_types.clear();
        capture_function_mangle_target(captured_target,
                                       qualified_name_label,
                                       captured_function);
      }
      out.swap(candidate);
      return true;
    }
    for(size_t i = 0; i < options.parameter_pattern->size(); ++i) {
      const size_t actual_index = i + param_start;
      TypePtr actual_param =
          actual_index < actual_function_type->params.size() ?
              actual_function_type->params[actual_index] :
              TypePtr();
      const CppAstNode * parameter_decl_pattern =
          options.parameter_declarations_pattern &&
                  i < options.parameter_declarations_pattern->size() ?
              (*options.parameter_declarations_pattern)[i] :
              nullptr;
      const bool parameter_decl_mentions_direct_template_parameter =
          parameter_decl_pattern &&
          ast_node_mentions_direct_template_parameter(*parameter_decl_pattern,
                                                      &mangle_ctx);
      const bool parameter_decl_mentions_function_parameter =
          parameter_decl_pattern &&
          parameter_declaration_type_mentions_function_parameter(
              *parameter_decl_pattern,
              &mangle_ctx);
      const TypePtr hybrid_param_type =
          hybridize_pattern_type_with_actual((*options.parameter_pattern)[i].second,
                                             actual_param,
                                             &mangle_ctx);
      const bool trailing_function_parameter_pack =
          options.has_trailing_function_parameter_pack &&
          i + 1 == options.parameter_pattern->size();
      const bool parameter_type_mentions_direct_template_parameter =
          type_mentions_function_template_parameter_slice(
              (*options.parameter_pattern)[i].second,
              &mangle_ctx);
      const bool emit_trailing_function_parameter_pack =
          trailing_function_parameter_pack &&
          parameter_type_mentions_direct_template_parameter;
      abi_mangle::SubstitutionKey pack_substitution_key;
      if(emit_trailing_function_parameter_pack) {
        abi_mangle::Type parameter_ir;
        if(try_build_type_ir(hybrid_param_type, &mangle_ctx, parameter_ir)) {
          abi_mangle::Type pack_ir =
              parameter_ir.kind == abi_mangle::Type::TK_PACK_EXPANSION ?
                  parameter_ir :
                  abi_mangle::Type::pack_expansion(std::move(parameter_ir));
          abi_mangle::make_type_substitution_key(pack_ir,
                                                 pack_substitution_key);
        }
        candidate += "Dp";
      }
      bool mangled_param = false;
      bool have_captured_param_type = false;
      unique_ptr<abi_mangle::Type> captured_param_type;
      if(have_captured_function) {
        captured_param_type.reset(new abi_mangle::Type);
      }
      if(trailing_function_parameter_pack &&
         !emit_trailing_function_parameter_pack &&
         actual_param &&
         !type_has_dependent_mangle_state(actual_param)) {
        if(parameter_decl_mentions_direct_template_parameter) {
          mangled_param = build_and_emit_parameter_declaration_ir_from_ast(
              *parameter_decl_pattern,
              actual_param,
              &mangle_ctx,
              state,
              candidate,
              captured_param_type.get());
          have_captured_param_type = mangled_param;
        }
        if(!mangled_param) {
          mangled_param = build_and_emit_type_ir(actual_param,
                                                 &mangle_ctx,
                                                 state,
                                                 candidate,
                                                 captured_param_type.get());
          have_captured_param_type = mangled_param;
        }
      }
      if(owner_template_only_pattern &&
         actual_param &&
         (!type_has_dependent_mangle_state(actual_param) ||
          type_has_concrete_template_id_spelling_for_mangling(actual_param, &mangle_ctx))) {
        mangled_param = build_and_emit_type_ir(actual_param,
                                               &mangle_ctx,
                                               state,
                                               candidate,
                                               captured_param_type.get());
        have_captured_param_type = mangled_param;
      }
      if(type_has_structured_dependent_qualified_member(hybrid_param_type)) {
        const size_t structured_begin = candidate.size();
        // Prefer preserved dependent template/member state over re-reading type
        // syntax; this keeps pack, alias, and substitution handling typed.
        mangled_param = build_and_emit_type_ir(hybrid_param_type,
                                               &mangle_ctx,
                                               state,
                                               candidate,
                                               captured_param_type.get());
        if(!mangled_param) {
          candidate.resize(structured_begin);
          have_captured_param_type = false;
        } else {
          have_captured_param_type = true;
        }
      }
      if(!mangled_param &&
         type_has_dependent_class_template_nested_owner_mangle_state(
             hybrid_param_type)) {
        const size_t structured_begin = candidate.size();
        mangled_param = build_and_emit_type_ir(hybrid_param_type,
                                               &mangle_ctx,
                                               state,
                                               candidate,
                                               captured_param_type.get());
        if(!mangled_param) {
          candidate.resize(structured_begin);
          have_captured_param_type = false;
        } else {
          have_captured_param_type = true;
        }
      }
      if(!mangled_param &&
         type_is_direct_template_parameter_ir(hybrid_param_type, &mangle_ctx)) {
        mangled_param =
            build_and_emit_type_ir(hybrid_param_type,
                                   &mangle_ctx,
                                   state,
                                   candidate,
                                   captured_param_type.get());
        have_captured_param_type = mangled_param;
      }
      if(!mangled_param &&
         parameter_decl_pattern &&
         (parameter_decl_mentions_direct_template_parameter ||
          parameter_decl_mentions_function_parameter)) {
        const size_t structured_begin = candidate.size();
        mangled_param = build_and_emit_parameter_declaration_ir_from_ast(
            *parameter_decl_pattern,
            actual_param,
            &mangle_ctx,
            state,
            candidate,
            captured_param_type.get());
        if(!mangled_param) {
          candidate.resize(structured_begin);
          if(actual_param && !type_has_dependent_mangle_state(actual_param) &&
             build_and_emit_type_ir(
                 actual_param,
                 &mangle_ctx,
                 state,
                 candidate,
                 captured_param_type.get())) {
            mangled_param = true;
            have_captured_param_type = true;
          } else {
            throw logic_error(
                string("dependent function parameter declaration reached unstructured ABI text mangler: ") +
                ast_node_diagnostic_label(*parameter_decl_pattern));
          }
        } else {
          have_captured_param_type = true;
        }
      }
      if(!mangled_param) {
        const size_t hybrid_begin = candidate.size();
        if(build_and_emit_type_ir(hybrid_param_type,
                                  &mangle_ctx,
                                  state,
                                  candidate,
                                  captured_param_type.get())) {
          mangled_param = true;
          have_captured_param_type = true;
        } else {
          candidate.resize(hybrid_begin);
          if(actual_param &&
             !type_has_dependent_mangle_state(actual_param) &&
             build_and_emit_type_ir(actual_param,
                                    &mangle_ctx,
                                    state,
                                    candidate,
                                    captured_param_type.get())) {
            mangled_param = true;
            have_captured_param_type = true;
          } else {
            return false;
          }
        }
      }
      if(!mangled_param) {
        have_captured_param_type = true;
      }
      if(!pack_substitution_key.empty()) {
        register_ir_substitution_key_owned(state, std::move(pack_substitution_key));
      }
	      if(have_captured_function && have_captured_param_type) {
	        if(emit_trailing_function_parameter_pack &&
	           captured_param_type->kind != abi_mangle::Type::TK_PACK_EXPANSION) {
          *captured_param_type =
              abi_mangle::Type::pack_expansion(std::move(*captured_param_type));
	          attach_context_free_type_ir_substitution(*captured_param_type);
	        }
	        captured_function.parameter_types.push_back(
	            std::move(*captured_param_type));
	      }
    }
    if((pattern_type && pattern_type->variadic) ||
       (!pattern_type && actual_function_type->variadic)) {
      candidate += 'z';
    }
    if(have_captured_function) {
      captured_function.variadic =
          (pattern_type && pattern_type->variadic) ||
          (!pattern_type && actual_function_type->variadic);
      capture_function_mangle_target(captured_target,
                                     qualified_name_label,
                                     captured_function);
    }
    out.swap(candidate);
    return true;
  }

  if(type->params.size() <= param_start) {
    candidate += type->variadic ? 'z' : 'v';
    if(have_captured_function) {
      captured_function.variadic = type->variadic;
      captured_function.parameter_types.clear();
      capture_function_mangle_target(captured_target,
                                     qualified_name_label,
                                     captured_function);
    }
    out.swap(candidate);
    return true;
  }

  for(size_t i = param_start; i < type->params.size(); ++i) {
    unique_ptr<abi_mangle::Type> captured_param_type;
    if(have_captured_function) {
      captured_param_type.reset(new abi_mangle::Type);
    }
	      if(try_emit_owner_type_parameter_pack_function_parameter_ir(type->params[i],
	                                                                  &mangle_ctx,
	                                                                  state,
	                                                                  candidate,
	                                                                  captured_param_type.get())) {
	        if(have_captured_function) {
	          captured_function.parameter_types.push_back(
	              std::move(*captured_param_type));
	        }
	        continue;
	      }
    if(!build_and_emit_type_ir(type->params[i],
                               &mangle_ctx,
                               state,
                               candidate,
                               captured_param_type.get())) {
      if(parser_trace::enabled("symbol.linkage")) {
        ostringstream trace;
        trace << "mangle-itanium-function parameter-type-failed"
              << " index=" << i
              << " display=" << type->params[i]->named_display
              << " key=" << type->params[i]->named_key
              << " kind=" << static_cast<int>(type->params[i]->kind)
              << " owner-pack="
              << (type_mentions_owner_type_template_parameter_pack(type->params[i],
                                                                    &mangle_ctx) ?
                      "yes" : "no");
        parser_trace::note("symbol.linkage", string(), trace.str());
      }
      return false;
    }
	    if(have_captured_function) {
	      captured_function.parameter_types.push_back(
	          std::move(*captured_param_type));
	    }
  }
  if(type->variadic) {
    candidate += 'z';
  }
  if(have_captured_function) {
    captured_function.variadic = type->variadic;
    capture_function_mangle_target(captured_target,
                                   qualified_name_label,
                                   captured_function);
  }
  out.swap(candidate);
  return true;
}

static bool try_emit_static_member_object_symbol_ir(
    const semantic_model::ClassInfo & owner_class,
    const string & member_name,
    string & out,
    const vector<TemplateArgument> * template_arguments,
    const vector<TemplateParameterInfo> * template_parameters)
{
  if(!owner_class.type || trim_space(member_name).empty()) {
    return false;
  }

  abi_mangle::Type owner;
  MangleSubstitutionState state;
  TypeMangleContext owner_ctx;
  owner_ctx.prefer_concrete_non_type_values_for_dependent_parameter_types = true;
  if(!try_build_type_ir(owner_class.type, &owner_ctx, owner)) {
    return false;
  }

  vector<abi_mangle::TemplateArgument> ir_template_arguments;
  const vector<abi_mangle::TemplateArgument> * ir_template_arguments_ptr = nullptr;
  if(template_arguments && !template_arguments->empty()) {
    if(!try_build_function_template_arguments_ir(*template_arguments,
                                                 template_parameters,
                                                 nullptr,
                                                 &owner_ctx,
                                                 ir_template_arguments)) {
      return false;
    }
    ir_template_arguments_ptr = &ir_template_arguments;
  }

  MangleIrSubstitutionSink sink(&state);
  string candidate;
  if(!abi_mangle::emit_external_member_entity_symbol(&owner,
                                                     member_name,
                                                     vector<abi_mangle::Type>(),
                                                     false,
                                                     false,
                                                     false,
                                                     false,
                                                     false,
                                                     false,
                                                     candidate,
                                                     &sink,
                                                     ir_template_arguments_ptr)) {
    return false;
  }
  out.swap(candidate);
  return true;
}

static bool try_emit_static_member_name_encoding_ir(
    const semantic_model::ClassInfo & owner_class,
    const string & member_name,
    string & out)
{
  if(!owner_class.type || trim_space(member_name).empty()) {
    return false;
  }

  abi_mangle::Type owner;
  MangleSubstitutionState state;
  TypeMangleContext owner_ctx;
  owner_ctx.prefer_concrete_non_type_values_for_dependent_parameter_types = true;
  if(!try_build_type_ir(owner_class.type, &owner_ctx, owner)) {
    return false;
  }

  MangleIrSubstitutionSink sink(&state);
  string candidate = "N";
  if(!abi_mangle::emit_type_as_name_prefix_body(owner, candidate, &sink) ||
     !abi_mangle::emit_source_name(member_name, candidate)) {
    return false;
  }
  candidate += 'E';
  out.swap(candidate);
  return true;
}

static bool is_named_class_member_scope(const semantic_model::Scope * scope)
{
  return scope &&
         scope->class_info &&
         scope->class_info->member_scope.get() == scope &&
         scope->name != "<unnamed>";
}

static bool is_simple_source_name_component(const string & text)
{
  if(text.empty()) {
    return false;
  }
  for(size_t i = 0; i < text.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    if((ch >= 'a' && ch <= 'z') ||
       (ch >= 'A' && ch <= 'Z') ||
       (ch >= '0' && ch <= '9') ||
       ch == '_') {
      continue;
    }
    return false;
  }
  return true;
}

static bool scoped_variable_symbol_parts(const semantic_model::Scope & scope,
                                         const string & name,
                                         vector<string> & out)
{
  if(!is_simple_source_name_component(name) ||
     name.compare(0, 10, "__builtin_") == 0) {
    return false;
  }

  out.clear();
  out.push_back(name);
  for(const semantic_model::Scope * current = &scope;
      current;
      current = current->parent) {
    if(current->name == "<global>") {
      continue;
    }
    if(current->namespace_scope) {
      const string component =
          current->name == "<unnamed>" ? string("_GLOBAL__N_1") : current->name;
      if(!is_simple_source_name_component(component)) {
        return false;
      }
      out.push_back(std::move(component));
      continue;
    }
    if(is_named_class_member_scope(current)) {
      return false;
    }
  }
  return true;
}

static string scoped_symbol_qualified_name_from_parts(const vector<string> & parts)
{
  string out;
  for(size_t i = parts.size(); i > 0; --i) {
    if(!out.empty()) {
      out += "::";
    }
    out += parts[i - 1];
  }
  return out;
}

static string scoped_symbol_qualified_name_from_scope(const semantic_model::Scope & scope,
                                                      const string & name)
{
  vector<string> parts;
  parts.push_back(name);
  for(const semantic_model::Scope * current = &scope;
      current;
      current = current->parent) {
    if(current->name == "<global>") {
      continue;
    }
    if(current->namespace_scope) {
      parts.push_back(current->name == "<unnamed>" ? string("_GLOBAL__N_1") :
                                                     current->name);
      continue;
    }
    if(is_named_class_member_scope(current)) {
      parts.push_back(current->name);
    }
  }
  return scoped_symbol_qualified_name_from_parts(parts);
}

static bool qualified_name_from_scoped_symbol_parts(const vector<string> & parts,
                                                    QualifiedName & out)
{
  if(parts.empty()) {
    return false;
  }
  out = QualifiedName();
  out.name = parts[0];
  for(size_t i = parts.size(); i > 1; --i) {
    out.qualifiers.push_back(parts[i - 1]);
  }
  return true;
}

static bool try_emit_scoped_variable_object_symbol_ir(const semantic_model::Scope & scope,
                                                   const string & name,
                                                   string & out,
                                                   string * qualified_name)
{
  vector<string> parts;
  if(!scoped_variable_symbol_parts(scope, name, parts)) {
    return false;
  }
  if(qualified_name) {
    *qualified_name = scoped_symbol_qualified_name_from_parts(parts);
  }

  QualifiedName qualified;
  return qualified_name_from_scoped_symbol_parts(parts, qualified) &&
         try_emit_qualified_name_object_symbol_ir(qualified, out);
}

string thread_local_wrapper_object_symbol_for_scoped_variable(
    const semantic_model::Scope & scope,
    const string & name)
{
  vector<string> parts;
  if(!scoped_variable_symbol_parts(scope, name, parts)) {
    return string();
  }
  QualifiedName qualified;
  string encoding;
  string out;
  if(!qualified_name_from_scoped_symbol_parts(parts, qualified) ||
     !try_emit_qualified_name_encoding_ir(qualified, encoding) ||
     !abi_mangle::emit_thread_local_wrapper_symbol_from_encoding(encoding, out)) {
    return string();
  }
  return out;
}

string thread_local_wrapper_object_symbol_for_static_member_variable(
    const semantic_model::ClassInfo & owner_class,
    const string & member_name)
{
  string encoding;
  string out;
  if(!try_emit_static_member_name_encoding_ir(owner_class, member_name, encoding) ||
     !abi_mangle::emit_thread_local_wrapper_symbol_from_encoding(encoding, out)) {
    return string();
  }
  return out;
}

bool mangle_itanium_type_encoding(const TypePtr & type, string & out)
{
  string candidate;
  MangleSubstitutionState state;
  if(!try_emit_type_encoding_ir_impl(type, candidate, nullptr, &state)) {
    return false;
  }
  out.swap(candidate);
  return true;
}

bool mangle_itanium_name_encoding(const QualifiedName & qualified_name, string & out)
{
  return try_emit_qualified_name_encoding_ir(qualified_name, out);
}

static bool type_needs_structural_internal_symbol_impl(
    const TypePtr & type,
    vector<const Type *> & active_types);

static bool template_argument_needs_structural_internal_symbol(
    const TemplateArgument & argument,
    vector<const Type *> & active_types)
{
  switch(argument.kind) {
  case TemplateArgument::TA_TYPE:
    return type_needs_structural_internal_symbol_impl(argument.type,
                                                      active_types);

  case TemplateArgument::TA_VALUE:
    return argument.function_value || argument.value_binding ||
           !argument.value_dependencies.empty();

  case TemplateArgument::TA_CLASS_TEMPLATE:
  case TemplateArgument::TA_ALIAS_TEMPLATE:
    return false;
  }
  return false;
}

static bool class_template_arguments_need_structural_internal_symbol(
    const TypePtr & type,
    vector<const Type *> & active_types)
{
  shared_ptr<const ClassTemplateSpecializationMangleInfo> info =
      named_type_class_template_specialization_mangle_info_const(type);
  if(!info) {
    return false;
  }
  for(size_t i = 0; i < info->arguments.size(); ++i) {
    if(template_argument_needs_structural_internal_symbol(info->arguments[i],
                                                          active_types)) {
      return true;
    }
  }
  return false;
}

static bool type_needs_structural_internal_symbol_impl(
    const TypePtr & type,
    vector<const Type *> & active_types)
{
  if(!type) {
    return false;
  }
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(find(active_types.begin(), active_types.end(), base.get()) !=
     active_types.end()) {
    return false;
  }

  switch(base->kind) {
  case Type::TK_POINTER:
  case Type::TK_MEMBER_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
  case Type::TK_FUNCTION:
    return true;

  case Type::TK_ATOMIC:
    return type_needs_structural_internal_symbol_impl(base->inner,
                                                      active_types);

  case Type::TK_NAMED:
    active_types.push_back(base.get());
    {
      const bool needs =
          class_template_arguments_need_structural_internal_symbol(base,
                                                                   active_types);
      active_types.pop_back();
      return needs;
    }

  case Type::TK_FUNDAMENTAL:
  case Type::TK_CV:
    return false;
  }
  return false;
}

bool type_needs_structural_internal_symbol(const TypePtr & type)
{
  vector<const Type *> active_types;
  return type_needs_structural_internal_symbol_impl(type, active_types);
}

string internal_symbol_from_type_encoding(const string & prefix,
                                          const TypePtr & type)
{
  string encoding;
  if(!mangle_itanium_type_encoding(type, encoding) || encoding.empty()) {
    return string();
  }
  return string("@") + mangle_symbol_name(prefix) + "_" +
         mangle_symbol_name(encoding);
}

static bool emit_itanium_function_encoding_with_substitutions(
    const QualifiedName & qualified_name,
    const string & display_name,
    const TypePtr & type,
    const FunctionSymbolOptions & options,
    string & out,
    vector<abi_mangle::SubstitutionSlot> * substitution_slots)
{
  string candidate;
  MangleSubstitutionState state;
  const string qualified_name_label = qualified_name_syntax_key_text(qualified_name);
  if(!try_emit_itanium_function_symbol_ir(qualified_name,
                                          qualified_name_label,
                                          display_name,
                                          type,
                                          options,
                                          candidate,
                                          substitution_slots ? &state : nullptr) ||
     !abi_mangle::object_symbol_body(candidate, out)) {
    return false;
  }
  if(substitution_slots) {
    *substitution_slots = substitution_slots_from_state(&state);
  }
  return true;
}

static bool uses_builtin_runtime_override(const string & name)
{
  const string compact = remove_space_chars(name);
  return compact.compare(0, 10, "__builtin_") == 0 ||
         compact == "__builtin_operator_new" ||
         compact == "__builtin_operator_delete";
}

static SymbolIdentity c_linkage_identity(const string & raw_name,
                                         SymbolLinkage linkage)
{
  SymbolIdentity out;
  out.internal_symbol = internal_symbol_from_name(raw_name);
  out.linkage = linkage;
  out.object_symbol = raw_name;
  return out;
}

static void append_abi_mangle_fact(SymbolIdentity & symbol,
                                   const string & object_symbol,
                                   const abi_mangle::AbiMangleTarget & target)
{
  if(object_symbol.empty() || target.kind == abi_mangle::ABI_MANGLE_NONE) {
    return;
  }
  SymbolIdentity::AbiMangleFactEntry entry;
  entry.object_symbol = object_symbol;
  entry.target = target;
  if(!symbol.abi_mangle_facts) {
    symbol.abi_mangle_facts.reset(
        new SymbolIdentity::AbiMangleFactEntries);
  }
  symbol.abi_mangle_facts->entries.push_back(entry);
}

static void append_c_function_abi_mangle_fact(SymbolIdentity & symbol,
                                              const string & qualified_name)
{
  if(!abi_mangle_fact_capture_enabled_ref()) {
    return;
  }
  abi_mangle::AbiMangleTarget target;
  target.kind = abi_mangle::ABI_MANGLE_FUNCTION;
  target.c_linkage = true;
  target.qualified_name = qualified_name;
  append_abi_mangle_fact(symbol, symbol.object_symbol, target);
}

static void append_function_abi_mangle_fact(SymbolIdentity & symbol,
                                            const abi_mangle::AbiMangleTarget & target)
{
  append_abi_mangle_fact(symbol, symbol.object_symbol, target);
}

static void append_variable_abi_mangle_fact(SymbolIdentity & symbol,
                                            const string & object_symbol,
                                            const string & qualified_name,
                                            bool is_c_linkage)
{
  if(!abi_mangle_fact_capture_enabled_ref()) {
    return;
  }
  abi_mangle::AbiMangleTarget target;
  target.kind = abi_mangle::ABI_MANGLE_VARIABLE;
  target.c_linkage = is_c_linkage;
  target.qualified_name = qualified_name;
  append_abi_mangle_fact(symbol, object_symbol, target);
}

static const char * linkage_name(SymbolLinkage linkage)
{
  switch(linkage) {
  case SL_INTERNAL:
    return "internal";
  case SL_EXTERNAL:
    return "external";
  case SL_WEAK:
    return "weak";
  }
  return "unknown";
}

static void note_symbol_linkage_event(const char * action,
                                      const string & entity,
                                      const string & display_name,
                                      bool is_c_linkage,
                                      const SymbolIdentity & symbol)
{
  if(!parser_trace::enabled("symbol.linkage")) {
    return;
  }

  ostringstream trace;
  trace << "action=" << action
        << " entity=" << entity
        << " display=" << display_name
        << " c-linkage=" << (is_c_linkage ? "yes" : "no")
        << " internal=" << symbol.internal_symbol
        << " object=" << symbol.object_symbol
        << " linkage=" << linkage_name(symbol.linkage)
        << " keep-alias=" << (symbol.keep_internal_alias ? "yes" : "no");
  parser_trace::note("symbol.linkage", string(), trace.str());
}

SymbolIdentity make_c_function_symbol_identity(const string & name,
                                               SymbolLinkage linkage)
{
  SymbolIdentity out = c_linkage_identity(name, linkage);
  append_c_function_abi_mangle_fact(out, name);
  note_symbol_linkage_event("function", name, name, true, out);
  return out;
}

SymbolIdentity make_function_symbol_identity(const QualifiedName & qualified,
                                             const string & display_name,
                                             bool is_c_linkage,
                                             const TypePtr & type,
                                             const FunctionSymbolOptions & options,
                                             const string & symbol_key,
                                             SymbolLinkage linkage)
{
  const string qualified_name = qualified_name_syntax_key_text(qualified);
  (void)symbol_key;
  if(is_c_linkage) {
    SymbolIdentity out = c_linkage_identity(display_name, linkage);
    append_c_function_abi_mangle_fact(out, qualified_name);
    note_symbol_linkage_event("function", qualified_name, display_name, true, out);
    return out;
  }

  SymbolIdentity out;
  out.internal_symbol = internal_symbol_from_name(qualified_name);
  out.linkage = linkage;
  bool emitted_object_symbol = false;
  const bool capture_abi_fact = abi_mangle_fact_capture_enabled_ref();
  abi_mangle::AbiMangleTarget emitted_abi_target;
  if(linkage == SL_INTERNAL) {
    if(!uses_builtin_runtime_override(display_name)) {
      emitted_object_symbol =
          try_emit_itanium_function_symbol_ir(qualified,
                                              qualified_name,
                                              display_name,
                                              type,
                                              options,
                                              out.object_symbol,
                                              nullptr,
                                              linkage != SL_WEAK,
                                              capture_abi_fact ? &emitted_abi_target :
                                                                 nullptr);
    }
  } else if(qualified_name == "main" && display_name == "main") {
    out.object_symbol = "main";
    out.keep_internal_alias = true;
  } else if(!uses_builtin_runtime_override(display_name) &&
            (linkage == SL_WEAK || linkage == SL_EXTERNAL)) {
    emitted_object_symbol =
        try_emit_itanium_function_symbol_ir(qualified,
                                            qualified_name,
                                            display_name,
                                            type,
                                            options,
                                            out.object_symbol,
                                            nullptr,
                                            linkage != SL_WEAK,
                                            capture_abi_fact ? &emitted_abi_target :
                                                               nullptr);
    if(!emitted_object_symbol && linkage == SL_WEAK) {
      throw logic_error("failed to build ABI IR function symbol for weak function " +
                        qualified_name);
    }
  }
  if(emitted_object_symbol && capture_abi_fact) {
    append_function_abi_mangle_fact(out, emitted_abi_target);
  }
  note_symbol_linkage_event("function", qualified_name, display_name, false, out);
  return out;
}

SymbolIdentity make_internal_symbol_identity(const string & internal_symbol,
                                             SymbolLinkage linkage)
{
  SymbolIdentity out;
  out.internal_symbol = internal_symbol;
  out.object_symbol = internal_symbol;
  out.linkage = linkage;
  return out;
}

SymbolIdentity make_object_symbol_identity(const string & internal_symbol,
                                           const string & object_symbol,
                                           SymbolLinkage linkage)
{
  SymbolIdentity out;
  out.internal_symbol = internal_symbol;
  out.object_symbol = object_symbol;
  out.linkage = linkage;
  return out;
}

SymbolIdentity make_scoped_variable_symbol_identity(const semantic_model::Scope & scope,
                                                    const string & name,
                                                    bool is_c_linkage,
                                                    SymbolLinkage linkage)
{
  string qualified_name = scoped_symbol_qualified_name_from_scope(scope, name);
  vector<string> parts;
  if(scoped_variable_symbol_parts(scope, name, parts)) {
    qualified_name = scoped_symbol_qualified_name_from_parts(parts);
  }

  if(is_c_linkage) {
    SymbolIdentity out = c_linkage_identity(name, linkage);
    append_variable_abi_mangle_fact(out, out.object_symbol, qualified_name, true);
    note_symbol_linkage_event("variable", qualified_name, name, true, out);
    return out;
  }

  SymbolIdentity out;
  out.internal_symbol = internal_symbol_from_name(qualified_name);
  out.linkage = linkage;
  if(!try_emit_scoped_variable_object_symbol_ir(scope,
                                                name,
                                                out.object_symbol,
                                                &qualified_name)) {
    throw logic_error("failed to mangle scoped variable symbol from semantic scope " +
                      qualified_name);
  }
  append_variable_abi_mangle_fact(out, out.object_symbol, qualified_name, false);
  note_symbol_linkage_event("variable", qualified_name, name, false, out);
  return out;
}

SymbolIdentity make_static_member_variable_symbol_identity(
    const semantic_model::ClassInfo & owner_class,
    const string & member_name,
    bool is_c_linkage,
    SymbolLinkage linkage)
{
  const string qualified_name =
      owner_class.qualified_name.empty() ?
          member_name :
          owner_class.qualified_name + "::" + member_name;
  if(is_c_linkage) {
    SymbolIdentity out = c_linkage_identity(member_name, linkage);
    append_variable_abi_mangle_fact(out, out.object_symbol, qualified_name, true);
    note_symbol_linkage_event("variable", qualified_name, member_name, true, out);
    return out;
  }

  SymbolIdentity out;
  out.internal_symbol = internal_symbol_from_name(qualified_name);
  out.linkage = linkage;
  if(!try_emit_static_member_object_symbol_ir(owner_class,
                                              member_name,
                                              out.object_symbol)) {
    throw logic_error("failed to mangle static member variable symbol from semantic owner " +
                      qualified_name);
  }
  append_variable_abi_mangle_fact(out, out.object_symbol, qualified_name, false);
  note_symbol_linkage_event("variable", qualified_name, member_name, false, out);
  return out;
}

SymbolIdentity make_static_member_variable_template_symbol_identity(
    const semantic_model::ClassInfo & owner_class,
    const string & internal_member_name,
    const string & template_name,
    const vector<TemplateArgument> & template_arguments,
    const vector<TemplateParameterInfo> & template_parameters,
    bool is_c_linkage,
    SymbolLinkage linkage)
{
  const string qualified_name =
      owner_class.qualified_name.empty() ?
          internal_member_name :
          owner_class.qualified_name + "::" + internal_member_name;
  if(is_c_linkage) {
    SymbolIdentity out = c_linkage_identity(internal_member_name, linkage);
    append_variable_abi_mangle_fact(out, out.object_symbol, qualified_name, true);
    note_symbol_linkage_event("variable",
                              qualified_name,
                              internal_member_name,
                              true,
                              out);
    return out;
  }

  SymbolIdentity out;
  out.internal_symbol = internal_symbol_from_name(qualified_name);
  out.linkage = linkage;
  if(!try_emit_static_member_object_symbol_ir(owner_class,
                                              template_name,
                                              out.object_symbol,
                                              &template_arguments,
                                              &template_parameters)) {
    throw logic_error("failed to mangle static member variable template symbol from semantic owner " +
                      qualified_name);
  }
  append_variable_abi_mangle_fact(out, out.object_symbol, qualified_name, false);
  note_symbol_linkage_event("variable",
                            qualified_name,
                            internal_member_name,
                            false,
                            out);
  return out;
}

bool has_external_vtable_symbol_candidate(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(base && base->kind == Type::TK_POINTER) {
    base = strip_top_level_cv(base->inner);
  }
  return !vtable_object_symbol_for_type(base).empty();
}

}  // namespace symbol_linkage
