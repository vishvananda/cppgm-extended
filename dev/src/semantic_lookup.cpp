#include "semantic_lookup.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "cpp_decl_bridge.h"
#include "cpp_scope_lookup.h"
#include "parser_trace.h"
#include "qualified_name_parser.h"
#include "semantic_class_model.h"
#include "semantic_context.h"
#include "semantic_dependent_type.h"
#include "semantic_metrics.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "template_api.h"

using namespace std;

namespace semantic_lookup {

using namespace cpp_decl;
using namespace template_model;

Scope * unqualified_friend_entity_scope(const ClassInfo & info)
{
  Scope * scope = info.enclosing_scope ?
      info.enclosing_scope :
      (info.member_scope ? info.member_scope->parent : nullptr);
  while(scope && !scope->namespace_scope && scope->parent) {
    scope = scope->parent;
  }
  return scope ? scope : (info.enclosing_scope ? info.enclosing_scope :
                                                info.member_scope.get());
}

namespace {

struct ParsedSourceLocation
{
  bool valid = false;
  string file;
  int line = 0;
  int column = 0;
};

ParsedSourceLocation parse_source_location(const string & text)
{
  ParsedSourceLocation parsed;
  const size_t last_colon = text.rfind(':');
  if(last_colon == string::npos) {
    return parsed;
  }
  const size_t second_colon = text.rfind(':', last_colon - 1);
  if(second_colon == string::npos) {
    return parsed;
  }
  parsed.file = text.substr(0, second_colon);
  parsed.line = std::atoi(text.substr(second_colon + 1,
                                      last_colon - second_colon - 1).c_str());
  parsed.column = std::atoi(text.substr(last_colon + 1).c_str());
  parsed.valid = !parsed.file.empty();
  return parsed;
}

string prefer_earlier_source_location(const string & first, const string & second)
{
  if(first.empty()) {
    return second;
  }
  if(second.empty()) {
    return first;
  }
  const ParsedSourceLocation parsed_first = parse_source_location(first);
  const ParsedSourceLocation parsed_second = parse_source_location(second);
  if(!parsed_first.valid || !parsed_second.valid ||
     parsed_first.file != parsed_second.file) {
    return first;
  }
  if(parsed_second.line < parsed_first.line) {
    return second;
  }
  if(parsed_second.line == parsed_first.line &&
     parsed_second.column < parsed_first.column) {
    return second;
  }
  return first;
}

Scope * root_scope(Scope & scope)
{
  Scope * current = &scope;
  while(current->parent) {
    current = current->parent;
  }
  return current;
}

bool is_named_class_member_scope(const Scope * scope)
{
  return scope &&
         scope->class_info &&
         scope->class_info->member_scope.get() == scope &&
         scope->name != "<unnamed>";
}

bool is_identifier_char(char ch)
{
  return (ch >= 'a' && ch <= 'z') ||
         (ch >= 'A' && ch <= 'Z') ||
         (ch >= '0' && ch <= '9') ||
         ch == '_';
}

const CppAstNode * first_identifier_node(const CppAstNode & node)
{
  if(node.kind == CppAstKind::identifier) {
    return &node;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(const CppAstNode * child = first_identifier_node(node.children[i])) {
      return child;
    }
  }
  return nullptr;
}

static string unqualified_name_text(const string & name)
{
  const size_t pos = semantic_utils::top_level_scope_split(name);
  return pos == string::npos ? name : name.substr(pos + 2);
}

static string qualified_name_text(const QualifiedName & name)
{
  ostringstream out;
  if(name.rooted) {
    out << "::";
  }
  for(size_t i = 0; i < name.qualifiers.size(); ++i) {
    out << name.qualifiers[i] << "::";
  }
  out << name.name;
  return out.str();
}

static string template_base_name(const string & name)
{
  const string unqualified = unqualified_name_text(name);
  const size_t pos = unqualified.find('<');
  return pos == string::npos ? unqualified : unqualified.substr(0, pos);
}

void hash_combine(size_t & seed, size_t value)
{
  seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

void hash_string_slice(size_t & seed,
                       const string & text,
                       size_t offset,
                       size_t length)
{
  size_t value = 1469598103934665603ULL;
  for(size_t i = 0; i < length; ++i) {
    value ^= static_cast<unsigned char>(text[offset + i]);
    value *= 1099511628211ULL;
  }
  hash_combine(seed, value);
}

bool string_slice_equal(const string & lhs,
                        size_t lhs_offset,
                        size_t lhs_length,
                        const string & rhs,
                        size_t rhs_offset,
                        size_t rhs_length)
{
  return lhs_length == rhs_length &&
         lhs.compare(lhs_offset, lhs_length, rhs, rhs_offset, rhs_length) == 0;
}

struct FunctionLookupNameParts
{
  size_t prefix_length = 0;
  size_t base_offset = 0;
  size_t base_length = 0;
  size_t suffix_offset = 0;
  size_t suffix_length = 0;
  bool operator_name = false;
};

FunctionLookupNameParts parse_function_lookup_name(const string & name)
{
  FunctionLookupNameParts out;
  const size_t scope_pos = semantic_utils::top_level_scope_split(name);
  out.prefix_length = scope_pos == string::npos ? 0 : scope_pos + 2;
  out.base_offset = out.prefix_length;
  out.base_length = name.size() - out.base_offset;
  if(out.base_length < 8 || name.compare(out.base_offset, 8, "operator") != 0) {
    return out;
  }

  out.operator_name = true;
  out.base_length = 8;
  size_t suffix_offset = out.base_offset + 8;
  while(suffix_offset < name.size() && name[suffix_offset] == ' ') {
    ++suffix_offset;
  }
  out.suffix_offset = suffix_offset;
  out.suffix_length = name.size() - suffix_offset;
  return out;
}

bool function_lookup_name_needs_canonicalization(const string & name)
{
  if(name.find("operator") == string::npos) {
    return false;
  }
  const FunctionLookupNameParts parts = parse_function_lookup_name(name);
  if(!parts.operator_name) {
    return false;
  }
  const size_t raw_suffix_offset = parts.base_offset + 8;
  return raw_suffix_offset != parts.suffix_offset;
}

bool function_lookup_name_parts_equal(const string & lhs,
                                      const FunctionLookupNameParts & lhs_parts,
                                      const string & rhs,
                                      const FunctionLookupNameParts & rhs_parts,
                                      bool compare_prefix)
{
  if(compare_prefix &&
     !string_slice_equal(lhs, 0, lhs_parts.prefix_length,
                         rhs, 0, rhs_parts.prefix_length)) {
    return false;
  }
  if(lhs_parts.operator_name || rhs_parts.operator_name) {
    return lhs_parts.operator_name &&
           rhs_parts.operator_name &&
           string_slice_equal(lhs,
                              lhs_parts.base_offset,
                              lhs_parts.base_length,
                              rhs,
                              rhs_parts.base_offset,
                              rhs_parts.base_length) &&
           string_slice_equal(lhs,
                              lhs_parts.suffix_offset,
                              lhs_parts.suffix_length,
                              rhs,
                              rhs_parts.suffix_offset,
                              rhs_parts.suffix_length);
  }
  return string_slice_equal(lhs,
                            lhs_parts.base_offset,
                            lhs_parts.base_length,
                            rhs,
                            rhs_parts.base_offset,
                            rhs_parts.base_length);
}

bool function_lookup_name_equal(const string & lhs, const string & rhs)
{
  if(lhs == rhs) {
    return true;
  }
  if(lhs.find("operator") == string::npos && rhs.find("operator") == string::npos) {
    return false;
  }
  return function_lookup_name_parts_equal(lhs,
                                          parse_function_lookup_name(lhs),
                                          rhs,
                                          parse_function_lookup_name(rhs),
                                          true);
}

bool unqualified_function_lookup_name_equal(const string & lhs, const string & rhs)
{
  if(lhs == rhs) {
    return true;
  }
  if(lhs.find("operator") == string::npos && rhs.find("operator") == string::npos) {
    const size_t lhs_scope_pos = semantic_utils::top_level_scope_split(lhs);
    const size_t rhs_scope_pos = semantic_utils::top_level_scope_split(rhs);
    const size_t lhs_offset = lhs_scope_pos == string::npos ? 0 : lhs_scope_pos + 2;
    const size_t rhs_offset = rhs_scope_pos == string::npos ? 0 : rhs_scope_pos + 2;
    return string_slice_equal(lhs, lhs_offset, lhs.size() - lhs_offset,
                              rhs, rhs_offset, rhs.size() - rhs_offset);
  }
  return function_lookup_name_parts_equal(lhs,
                                          parse_function_lookup_name(lhs),
                                          rhs,
                                          parse_function_lookup_name(rhs),
                                          false);
}

size_t canonical_unqualified_function_lookup_name_hash(const string & name)
{
  size_t seed = 0;
  const FunctionLookupNameParts parts = parse_function_lookup_name(name);
  if(parts.operator_name) {
    hash_string_slice(seed, name, parts.base_offset, parts.base_length);
    hash_string_slice(seed, name, parts.suffix_offset, parts.suffix_length);
  } else {
    hash_string_slice(seed, name, parts.base_offset, parts.base_length);
  }
  return seed;
}

size_t canonical_function_lookup_name_hash(const string & name)
{
  size_t seed = 0;
  const FunctionLookupNameParts parts = parse_function_lookup_name(name);
  hash_string_slice(seed, name, 0, parts.prefix_length);
  if(parts.operator_name) {
    hash_string_slice(seed, name, parts.base_offset, parts.base_length);
    hash_string_slice(seed, name, parts.suffix_offset, parts.suffix_length);
  } else {
    hash_string_slice(seed, name, parts.base_offset, parts.base_length);
  }
  return seed;
}

size_t type_lookup_hash(const TypePtr & type)
{
  if(!type) {
    return 0;
  }

  size_t seed = static_cast<size_t>(type->kind) + 1;
  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    hash_combine(seed, static_cast<size_t>(type->fundamental));
    break;

  case Type::TK_NAMED:
    hash_combine(seed, std::hash<string>()(type->named_key));
    hash_combine(seed, type->named_complete ? 1 : 0);
    break;

  case Type::TK_CV:
    hash_combine(seed, type->cv_const ? 1 : 0);
    hash_combine(seed, type->cv_volatile ? 1 : 0);
    hash_combine(seed, type_lookup_hash(type->inner));
    break;

  case Type::TK_ATOMIC:
    hash_combine(seed, type_lookup_hash(type->inner));
    break;

  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    hash_combine(seed, type_lookup_hash(type->inner));
    break;

  case Type::TK_MEMBER_POINTER:
    hash_combine(seed, type_lookup_hash(type->owner));
    hash_combine(seed, type_lookup_hash(type->inner));
    break;

  case Type::TK_ARRAY:
    hash_combine(seed, type->has_bound ? 1 : 0);
    hash_combine(seed, type->bound);
    hash_combine(seed, std::hash<string>()(type->bound_text));
    hash_combine(seed, type_lookup_hash(type->inner));
    break;

  case Type::TK_FUNCTION:
    hash_combine(seed, type->variadic ? 1 : 0);
    hash_combine(seed, type->prototype_relaxed ? 1 : 0);
    hash_combine(seed, type->function_const ? 1 : 0);
    hash_combine(seed, type->function_volatile ? 1 : 0);
    hash_combine(seed, type_lookup_hash(type->inner));
    hash_combine(seed, type->params.size());
    for(size_t i = 0; i < type->params.size(); ++i) {
      hash_combine(seed, type_lookup_hash(type->params[i]));
    }
    break;
  }
  return seed;
}

struct FunctionLookupDedupeKey
{
  size_t name_hash = 0;
  size_t type_hash = 0;
};

FunctionLookupDedupeKey function_lookup_dedupe_key(const FunctionBinding * binding)
{
  FunctionLookupDedupeKey out;
  if(!binding) {
    return out;
  }
  if(binding->cached_lookup_dedupe_key_valid &&
     binding->cached_lookup_dedupe_type == binding->type.get() &&
     binding->cached_lookup_dedupe_name_size == binding->name.size()) {
    out.name_hash = binding->cached_lookup_dedupe_name_hash;
    out.type_hash = binding->cached_lookup_dedupe_type_hash;
    return out;
  }
  out.name_hash = canonical_unqualified_function_lookup_name_hash(binding->name);
  out.type_hash = type_lookup_hash(binding->type);
  binding->cached_lookup_dedupe_name_hash = out.name_hash;
  binding->cached_lookup_dedupe_type_hash = out.type_hash;
  binding->cached_lookup_dedupe_type = binding->type.get();
  binding->cached_lookup_dedupe_name_size = binding->name.size();
  binding->cached_lookup_dedupe_key_valid = true;
  return out;
}

bool function_lookup_dedupe_key_maybe_equal(const FunctionLookupDedupeKey & lhs,
                                            const FunctionLookupDedupeKey & rhs)
{
  return lhs.name_hash == rhs.name_hash && lhs.type_hash == rhs.type_hash;
}

struct FunctionTemplateLookupDedupeKey
{
  size_t name_hash = 0;
  size_t parameter_count = 0;
  size_t flags = 0;
  int type_kind = -1;
};

FunctionTemplateLookupDedupeKey function_template_lookup_dedupe_key(
    const FunctionTemplateDecl * decl)
{
  FunctionTemplateLookupDedupeKey out;
  if(!decl) {
    return out;
  }
  out.name_hash = canonical_function_lookup_name_hash(decl->name);
  out.parameter_count = decl->parameters.size();
  if(decl->is_constructor) {
    out.flags |= 1u << 0;
  }
  if(decl->is_destructor) {
    out.flags |= 1u << 1;
  }
  if(decl->is_static_member) {
    out.flags |= 1u << 2;
  }
  if(decl->is_const_method) {
    out.flags |= 1u << 3;
  }
  if(decl->is_volatile_method) {
    out.flags |= 1u << 4;
  }
  out.flags |= static_cast<size_t>(decl->ref_qualifier) << 5;
  out.type_kind = decl->type_pattern ? static_cast<int>(decl->type_pattern->kind) : -1;
  return out;
}

bool function_template_lookup_dedupe_key_maybe_equal(
    const FunctionTemplateLookupDedupeKey & lhs,
    const FunctionTemplateLookupDedupeKey & rhs)
{
  return lhs.name_hash == rhs.name_hash &&
         lhs.parameter_count == rhs.parameter_count &&
         lhs.flags == rhs.flags &&
         lhs.type_kind == rhs.type_kind;
}

bool is_simple_identifier_text(const string & text)
{
  if(text.empty()) {
    return false;
  }
  if(text[0] >= '0' && text[0] <= '9') {
    return false;
  }
  for(size_t i = 0; i < text.size(); ++i) {
    if(!is_identifier_char(text[i])) {
      return false;
    }
  }
  return true;
}

bool has_top_level_declarator_syntax(const string & text)
{
  int angle_depth = 0;
  int paren_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(ch == '(') {
      ++paren_depth;
      continue;
    }
    if(ch == ')' && paren_depth > 0) {
      --paren_depth;
      continue;
    }
    if(angle_depth != 0 || paren_depth != 0) {
      continue;
    }
    if(ch == '*' || ch == '&' || ch == '[' || ch == ']') {
      return true;
    }
  }
  return false;
}

static string strip_friend_type_prefixes(const string & name)
{
  string stripped = name;
  const char * prefixes[] = {
      "dependent type ",
      "dependent alias ",
      "class ",
      "struct ",
      "union ",
      "enum "
  };
  for(size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const string prefix(prefixes[i]);
    if(stripped.compare(0, prefix.size(), prefix) == 0) {
      stripped.erase(0, prefix.size());
      break;
    }
  }
  return stripped;
}

static string strip_leading_typename_token(const string & text)
{
  const string trimmed = semantic_utils::trim_space(text);
  static const string keyword = "typename";
  if(trimmed.size() <= keyword.size() ||
     trimmed.compare(0, keyword.size(), keyword) != 0) {
    return trimmed;
  }
  const char next = trimmed[keyword.size()];
  if(next != ' ' && next != '\t' && next != '\n' && next != '\r') {
    return trimmed;
  }
  return semantic_utils::trim_space(trimmed.substr(keyword.size()));
}

bool friend_owner_matches_current_class(const ClassInfo * current_class,
                                        const string & owner_name)
{
  if(!current_class || owner_name.empty()) {
    return false;
  }

  if(current_class->source_template &&
     (current_class->source_template->name == owner_name ||
      template_base_name(current_class->source_template->name) ==
          template_base_name(owner_name))) {
    return true;
  }
  if(current_class->name == owner_name || current_class->qualified_name == owner_name) {
    return true;
  }
  if(template_base_name(current_class->name) == template_base_name(owner_name) ||
     template_base_name(current_class->qualified_name) == template_base_name(owner_name)) {
    return true;
  }
  return false;
}

bool friend_member_alias_resolves_to_current_class(const ClassInfo * current_class,
                                                   const string & member_name)
{
  if(!current_class || !current_class->member_scope || !current_class->type ||
     member_name.empty()) {
    return false;
  }

  map<string, TypePtr>::const_iterator found =
      current_class->member_scope->named_types.find(member_name);
  if(found == current_class->member_scope->named_types.end()) {
    return false;
  }

  TypePtr current_type = strip_top_level_cv(remove_reference_type(current_class->type));
  TypePtr alias_type = strip_top_level_cv(remove_reference_type(found->second));
  return current_type && alias_type && type_equals(current_type, alias_type);
}

MemberAccess named_type_access_for_lookup(const Scope & scope, const string & name)
{
  map<string, MemberAccess>::const_iterator found = scope.named_type_access.find(name);
  return found == scope.named_type_access.end() ? MA_PUBLIC : found->second;
}

const Scope * next_inline_namespace_collapsed_scope_component(const Scope * scope)
{
  for(const Scope * current = scope; current; current = current->parent) {
    if(current->parent == nullptr) {
      return nullptr;
    }
    if(!current->name.empty() && !current->inline_namespace) {
      return current;
    }
  }
  return nullptr;
}

bool same_inline_namespace_collapsed_scope_identity(const Scope * lhs,
                                                    const Scope * rhs)
{
  if(lhs == rhs) {
    return true;
  }

  const Scope * lhs_cursor = lhs;
  const Scope * rhs_cursor = rhs;
  while(true) {
    const Scope * lhs_component =
        next_inline_namespace_collapsed_scope_component(lhs_cursor);
    const Scope * rhs_component =
        next_inline_namespace_collapsed_scope_component(rhs_cursor);
    if(!lhs_component || !rhs_component) {
      return lhs_component == rhs_component;
    }
    if(lhs_component->name != rhs_component->name) {
      return false;
    }
    lhs_cursor = lhs_component->parent;
    rhs_cursor = rhs_component->parent;
  }
}

bool same_effective_function_entity_name(const FunctionBinding & lhs,
                                         const FunctionBinding & rhs)
{
  return unqualified_function_lookup_name_equal(lhs.name, rhs.name) &&
         same_inline_namespace_collapsed_scope_identity(lhs.declaration_scope,
                                                       rhs.declaration_scope);
}

string normalize_destructor_member_lookup_name(SemanticContext & ctx,
                                               Scope & scope,
                                               ClassInfo & object_class,
                                               const string & name)
{
  const string unqualified = unqualified_name_text(name);
  if(unqualified.size() <= 1 || unqualified[0] != '~' || !object_class.type) {
    return name;
  }

  const string target_name = unqualified.substr(1);
  const string object_unique_name = unqualified_name_text(object_class.qualified_name);
  if(!object_unique_name.empty() && target_name == object_unique_name) {
    const size_t scope_pos = semantic_utils::top_level_scope_split(name);
    const string prefix = scope_pos == string::npos ? string() : name.substr(0, scope_pos + 2);
    return prefix + "~" + object_class.name;
  }

  TypePtr target_type;
  for(Scope * current = &scope; current; current = current->parent) {
    map<string, TypePtr>::const_iterator direct = current->named_types.find(target_name);
    if(direct != current->named_types.end()) {
      target_type = direct->second;
      break;
    }
  }
  if(!target_type) {
    target_type = ctx.lookup_type(scope, target_name, true);
  }
  TypePtr object_type = strip_top_level_cv(remove_reference_type(object_class.type));
  const auto target_matches_object =
      [&](TypePtr candidate) -> bool
  {
    if(!candidate || !object_type) {
      return false;
    }
    candidate = strip_top_level_cv(remove_reference_type(candidate));
    if(candidate && type_equals(candidate, object_type)) {
      return true;
    }
    TypePtr resolved_target;
    if(semantic_dependent_type::resolve_instantiated_dependent_type(
           ctx,
           scope,
           candidate,
           resolved_target) &&
       resolved_target) {
      resolved_target = strip_top_level_cv(remove_reference_type(resolved_target));
      return resolved_target && type_equals(resolved_target, object_type);
    }
    return false;
  };

  if(!target_matches_object(target_type)) {
    TypePtr lookup_type = ctx.lookup_non_template_type_name(scope, target_name);
    if(!target_matches_object(lookup_type)) {
      lookup_type = ctx.lookup_type(scope, target_name, true);
    }
    if(!target_matches_object(lookup_type)) {
      return name;
    }
  }

  if(!object_type) {
    return name;
  }

  const size_t scope_pos = semantic_utils::top_level_scope_split(name);
  const string prefix = scope_pos == string::npos ? string() : name.substr(0, scope_pos + 2);
  return prefix + "~" + object_class.name;
}

string describe_class_candidate_list(const vector<ClassInfo *> & classes)
{
  ostringstream out;
  for(size_t i = 0; i < classes.size(); ++i) {
    if(i != 0) {
      out << "; ";
    }
    out << (classes[i] ? classes[i]->qualified_name : string("<null>"));
  }
  return out.str();
}

bool member_lookup_present(const MemberValueLookupResult & result)
{
  return result.binding != nullptr;
}

bool member_lookup_present(const MemberFunctionLookupResult & result)
{
  return !result.functions.empty();
}

bool member_lookup_present(const MemberFunctionTemplateLookupResult & result)
{
  return !result.templates.empty();
}

bool member_lookup_present(const MemberCallableLookupResult & result)
{
  return !result.functions.empty() || !result.templates.empty();
}

bool member_lookup_present(const MemberClassTemplateLookupResult & result)
{
  return result.class_template != nullptr;
}

bool member_lookup_present(const MemberAliasTemplateLookupResult & result)
{
  return result.alias_template != nullptr;
}

bool member_lookup_present(const MemberVariableTemplateLookupResult & result)
{
  return result.variable_template != nullptr;
}

bool is_enclosing_current_specialization_type(Scope & scope,
                                              const TypePtr & type)
{
  if(!type) {
    return false;
  }
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->class_info &&
       current->class_info->type &&
       type_equals(current->class_info->type, type)) {
      return true;
    }
  }
  return false;
}

template<typename Result, typename DirectLookup, typename Present>
Result lookup_unqualified_with_present(Scope & scope,
                                       const string & name,
                                       const DirectLookup & direct_lookup,
                                       const Present & present)
{
  return cpp_scope_lookup::lookup_unqualified<Result>(
      scope, name, direct_lookup, present);
}

bool same_inline_namespace_template_parameter_list(
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const std::vector<TemplateParameterInfo> & rhs_parameters)
{
  if(lhs_parameters.size() != rhs_parameters.size()) {
    return false;
  }
  for(std::size_t i = 0; i < lhs_parameters.size(); ++i) {
    if(lhs_parameters[i].kind != rhs_parameters[i].kind ||
       lhs_parameters[i].parameter_pack != rhs_parameters[i].parameter_pack ||
       lhs_parameters[i].template_parameter_count !=
           rhs_parameters[i].template_parameter_count) {
      return false;
    }
    if(lhs_parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE &&
       !type_equals(lhs_parameters[i].value_type, rhs_parameters[i].value_type)) {
      return false;
    }
  }
  return true;
}

bool same_inline_namespace_class_template_entity(const ClassTemplateDecl * lhs,
                                                 const ClassTemplateDecl * rhs)
{
  if(lhs == rhs) {
    return true;
  }
  if(!lhs || !rhs || lhs->name != rhs->name) {
    return false;
  }
  if(!same_inline_namespace_collapsed_scope_identity(lhs->declaring_scope,
                                                    rhs->declaring_scope)) {
    return false;
  }
  return same_inline_namespace_template_parameter_list(lhs->parameters, rhs->parameters);
}

bool same_inline_namespace_alias_template_entity(const AliasTemplateDecl * lhs,
                                                 const AliasTemplateDecl * rhs)
{
  if(lhs == rhs) {
    return true;
  }
  if(!lhs || !rhs || lhs->name != rhs->name) {
    return false;
  }
  if(!same_inline_namespace_collapsed_scope_identity(lhs->declaring_scope,
                                                    rhs->declaring_scope)) {
    return false;
  }
  if(!same_inline_namespace_template_parameter_list(lhs->parameters, rhs->parameters)) {
    return false;
  }
  if(lhs->resolved_type_pattern && rhs->resolved_type_pattern &&
     !type_equals(lhs->resolved_type_pattern, rhs->resolved_type_pattern)) {
    return false;
  }
  return true;
}

template<typename DeclT, typename DirectLookup, typename SameEntity>
void collect_decl_lookup_from_using_directives(Scope & scope,
                                               const std::string & name,
                                               std::set<const Scope *> & visited,
                                               const DirectLookup & direct_lookup,
                                               const SameEntity & same_entity,
                                               bool & found,
                                               DeclT *& value,
                                               bool & ambiguous)
{
  if(!visited.insert(&scope).second) {
    return;
  }

  for(std::size_t i = 0; i < scope.using_directives.size(); ++i) {
    Scope * imported = scope.using_directives[i];
    DeclT * direct = direct_lookup(*imported, name);
    if(direct) {
      if(!found) {
        found = true;
        value = direct;
      } else if(!same_entity(value, direct)) {
        ambiguous = true;
        return;
      }
    }

    collect_decl_lookup_from_using_directives(
        *imported,
        name,
        visited,
        direct_lookup,
        same_entity,
        found,
        value,
        ambiguous);
    if(ambiguous) {
      return;
    }
  }
}

template<typename DeclT, typename DirectLookup, typename SameEntity>
DeclT * lookup_unqualified_decl_with_entity_equivalence(
    Scope & scope,
    const std::string & name,
    const DirectLookup & direct_lookup,
    const SameEntity & same_entity)
{
  for(Scope * current = &scope; current; current = current->parent) {
    bool found_at_level = false;
    bool ambiguous_at_level = false;
    DeclT * result_at_level = nullptr;
    DeclT * direct = direct_lookup(*current, name);
    if(direct) {
      if(!current->namespace_scope) {
        return direct;
      }
      found_at_level = true;
      result_at_level = direct;
    }

    std::set<const Scope *> visited;
    collect_decl_lookup_from_using_directives(
        *current,
        name,
        visited,
        direct_lookup,
        same_entity,
        found_at_level,
        result_at_level,
        ambiguous_at_level);
    if(ambiguous_at_level) {
      return nullptr;
    }
    if(found_at_level) {
      return result_at_level;
    }
  }

  return nullptr;
}

template<typename Result, typename FinalLookup, typename Present>
Result lookup_qualified_with_present(Scope & scope,
                                     const QualifiedName & qualified,
                                     const FinalLookup & final_lookup,
                                     const Present & present)
{
  return cpp_scope_lookup::lookup_qualified<Result>(
      *root_scope(scope), qualified,
      [&scope, &present](const string & name) -> Scope *
      {
        return lookup_unqualified_with_present<Scope *>(
            scope, name,
            [](Scope & target, const string & lookup_name) -> Scope *
            {
              return resolve_direct_namespace(target, lookup_name);
            },
            present);
      },
      [](Scope & target, const string & lookup_name) -> Scope *
      {
        return resolve_direct_namespace(target, lookup_name);
      },
      final_lookup);
}

template<typename Callback>
size_t collect_base_paths_impl(const ClassInfo & current,
                               const ClassInfo * target,
                               size_t offset,
                               MemberAccess access,
                               set<const ClassInfo *> & visited_virtual,
                               const Callback & callback)
{
  size_t matches = 0;
  if(!target) {
    return matches;
  }
  if(!current.complete_subobjects.empty()) {
    for(size_t i = 0; i < current.complete_subobjects.size(); ++i) {
      const SubobjectInfo & subobject = current.complete_subobjects[i];
      if(subobject.type != target) {
        continue;
      }
      callback(offset + subobject.offset,
               combine_member_access(access, subobject.access));
      ++matches;
    }
    return matches;
  }
  if(&current == target) {
    callback(offset, access);
    ++matches;
  }

  for(size_t i = 0; i < current.bases.size(); ++i) {
    const BaseInfo & base = current.bases[i];
    if(base.is_virtual && !visited_virtual.insert(base.type).second) {
      continue;
    }
    matches += collect_base_paths_impl(*base.type,
                                       target,
                                       offset + base.offset,
                                       combine_member_access(access, base.access),
                                       visited_virtual,
                                       callback);
  }
  return matches;
}

template<typename Callback>
size_t collect_base_paths(const ClassInfo & current,
                          const ClassInfo * target,
                          size_t offset,
                          MemberAccess access,
                          const Callback & callback)
{
  set<const ClassInfo *> visited_virtual;
  return collect_base_paths_impl(current, target, offset, access, visited_virtual, callback);
}

template<typename Result, typename DirectLookup>
Result lookup_member_in_hierarchy(ClassInfo & info,
                                  const DirectLookup & direct_lookup)
{
  Result direct = direct_lookup(info);
  if(member_lookup_present(direct)) {
    return direct;
  }

  set<ClassInfo *> visited_virtual;
  vector<ClassInfo *> candidates;
  vector<MemberAccess> candidate_access;
  vector<ClassInfo *> stack;
  stack.push_back(&info);
  vector<MemberAccess> access_stack(1, MA_PUBLIC);
  while(!stack.empty()) {
    ClassInfo * current = stack.back();
    MemberAccess current_access = access_stack.back();
    stack.pop_back();
    access_stack.pop_back();

    for(size_t i = 0; i < current->bases.size(); ++i) {
      BaseInfo & base = current->bases[i];
      if(base.is_virtual && !visited_virtual.insert(base.type).second) {
        continue;
      }
      Result inherited = direct_lookup(*base.type);
      if(member_lookup_present(inherited)) {
        candidates.push_back(base.type);
        candidate_access.push_back(
            combine_member_access(current_access, base.access));
      } else {
        stack.push_back(base.type);
        access_stack.push_back(
            combine_member_access(current_access, base.access));
      }
    }
  }

  if(candidates.empty()) {
    return Result();
  }

  ClassInfo * declared_in = candidates[0];
  MemberAccess path_access = candidate_access[0];
  for(size_t i = 1; i < candidates.size(); ++i) {
    if(candidates[i] != declared_in) {
      ostringstream out;
      out << "ambiguous member lookup";
      out << " [root " << info.qualified_name << "]";
      out << " [candidates " << describe_class_candidate_list(candidates) << "]";
      throw logic_error(out.str());
    }
    path_access = combine_member_access(path_access, candidate_access[i]);
  }

  size_t path_offset = 0;
  MemberAccess resolved_access = MA_PUBLIC;
  if(!find_unique_base_path(info, declared_in, path_offset, resolved_access)) {
    return Result();
  }

  Result out = direct_lookup(*declared_in);
  if(!member_lookup_present(out)) {
    return Result();
  }
  out.declared_in = declared_in;
  out.path_access = combine_member_access(path_access, resolved_access);
  out.path_offset = path_offset;
  return out;
}

bool scope_is_within(const Scope & scope, const Scope * ancestor)
{
  for(const Scope * current = &scope; current; current = current->parent) {
    if(current == ancestor) {
      return true;
    }
  }
  return false;
}

bool is_self_full_collection_scope(Scope & scope, ClassInfo & info)
{
  return info.full_member_collection_in_progress &&
         info.member_scope &&
         scope_is_within(scope, info.member_scope.get());
}

bool is_concrete_class_template_qualifier(SemanticContext & ctx,
                                          const ClassInfo * qualifier_info)
{
  return qualifier_info &&
         qualifier_info->source_template &&
         template_arguments_fully_bind_parameters(
             qualifier_info->source_template->parameters,
             qualifier_info->instantiation_arguments) &&
         !template_arguments_are_dependent(
             qualifier_info->instantiation_arguments,
             [&ctx](const cpp_decl::TypePtr & type)
             {
               return ctx.type_depends_on_template_parameter(type);
             });
}

void ensure_class_reference_members_if_needed(SemanticContext & ctx,
                                              Scope & scope,
                                              ClassInfo & info)
{
  if(info.reference_members_collected ||
     info.reference_member_collection_in_progress ||
     is_self_full_collection_scope(scope, info)) {
    return;
  }
  ctx.ensure_class_reference_members(info);
}

template<typename Result, typename Lookup, typename Present>
Result lookup_inline_namespace_children(Scope & scope,
                                        const Lookup & lookup,
                                        const Present & present)
{
  for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
    Scope & child = *scope.namespace_children[i];
    if(!child.inline_namespace && child.name != "<unnamed>") {
      continue;
    }
    Result direct = lookup(child);
    if(present(direct)) {
      return direct;
    }
    Result nested = lookup_inline_namespace_children<Result>(child, lookup, present);
    if(present(nested)) {
      return nested;
    }
  }
  return Result();
}

ClassTemplateDecl * lookup_class_template_in_direct_or_inline_scope(Scope & scope,
                                                                    const string & name)
{
  if(ClassTemplateDecl * direct = lookup_direct_class_template(scope, name)) {
    return direct;
  }
  return lookup_inline_namespace_children<ClassTemplateDecl *>(
      scope,
      [&name](Scope & child) -> ClassTemplateDecl *
      {
        return lookup_direct_class_template(child, name);
      },
      [](ClassTemplateDecl * decl) -> bool
      {
        return decl != nullptr;
      });
}

AliasTemplateDecl * lookup_alias_template_in_direct_or_inline_scope(Scope & scope,
                                                                    const string & name)
{
  if(AliasTemplateDecl * direct = lookup_direct_alias_template(scope, name)) {
    return direct;
  }
  return lookup_inline_namespace_children<AliasTemplateDecl *>(
      scope,
      [&name](Scope & child) -> AliasTemplateDecl *
      {
        return lookup_direct_alias_template(child, name);
      },
      [](AliasTemplateDecl * decl) -> bool
      {
        return decl != nullptr;
      });
}

template<typename DeclT, typename MapLookup>
DeclT * lookup_inherited_member_template_decl(SemanticContext & ctx,
                                              ClassInfo & info,
                                              const string & name,
                                              const MapLookup & map_lookup)
{
  set<ClassInfo *> visited_virtual;
  vector<ClassInfo *> candidates;
  vector<ClassInfo *> stack;
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].type) {
      stack.push_back(info.bases[i].type);
    }
  }

  while(!stack.empty()) {
    ClassInfo * current = stack.back();
    stack.pop_back();
    if(!current || current->reference_member_collection_in_progress) {
      continue;
    }
    if(!current->reference_members_collected) {
      ctx.ensure_class_reference_members(*current);
    }
    if(current->member_scope && map_lookup(*current->member_scope, name)) {
      candidates.push_back(current);
      continue;
    }
    for(size_t i = 0; i < current->bases.size(); ++i) {
      BaseInfo & base = current->bases[i];
      if(base.is_virtual && !visited_virtual.insert(base.type).second) {
        continue;
      }
      if(base.type) {
        stack.push_back(base.type);
      }
    }
  }

  if(candidates.empty()) {
    return nullptr;
  }
  ClassInfo * declared_in = candidates[0];
  for(size_t i = 1; i < candidates.size(); ++i) {
    if(candidates[i] != declared_in) {
      ostringstream out;
      out << "ambiguous member template lookup";
      out << " [root " << info.qualified_name << "]";
      out << " [name " << name << "]";
      out << " [candidates " << describe_class_candidate_list(candidates) << "]";
      throw logic_error(out.str());
    }
  }
  return declared_in && declared_in->member_scope ?
      map_lookup(*declared_in->member_scope, name) :
      nullptr;
}

ClassTemplateDecl * lookup_class_template_in_scope_or_inherited_members(
    SemanticContext & ctx,
    Scope & scope,
    const string & name)
{
  if(ClassTemplateDecl * direct = lookup_direct_class_template(scope, name)) {
    return direct;
  }
  if(!scope.class_info) {
    return nullptr;
  }
  return lookup_inherited_member_template_decl<ClassTemplateDecl>(
      ctx,
      *scope.class_info,
      name,
      [](Scope & target, const string & lookup_name) -> ClassTemplateDecl *
      {
        return lookup_direct_class_template(target, lookup_name);
      });
}

AliasTemplateDecl * lookup_alias_template_in_scope_or_inherited_members(
    SemanticContext & ctx,
    Scope & scope,
    const string & name)
{
  if(AliasTemplateDecl * direct = lookup_direct_alias_template(scope, name)) {
    return direct;
  }
  if(!scope.class_info) {
    return nullptr;
  }
  return lookup_inherited_member_template_decl<AliasTemplateDecl>(
      ctx,
      *scope.class_info,
      name,
      [](Scope & target, const string & lookup_name) -> AliasTemplateDecl *
      {
        return lookup_direct_alias_template(target, lookup_name);
      });
}

template<typename DeclT, typename DirectLookup, typename SameEntity>
DeclT * lookup_qualified_decl_with_using_directives(
    Scope & scope,
    const string & name,
    const DirectLookup & direct_lookup,
    const SameEntity & same_entity)
{
  DeclT * direct = direct_lookup(scope, name);
  if(scope.using_directives.empty()) {
    return direct;
  }
  bool found = false;
  bool ambiguous = false;
  DeclT * imported = nullptr;
  std::set<const Scope *> visited;
  collect_decl_lookup_from_using_directives<DeclT>(
      scope,
      name,
      visited,
      direct_lookup,
      same_entity,
      found,
      imported,
      ambiguous);
  if(ambiguous) {
    return nullptr;
  }
  if(direct && imported && !same_entity(direct, imported)) {
    return nullptr;
  }
  return direct ? direct : imported;
}

bool namespace_child_injected_for_direct_lookup(const Scope & child)
{
  return child.inline_namespace || child.name == "<unnamed>";
}

TypePtr resolve_direct_type_qualifier_local(SemanticContext & ctx,
                                            Scope & scope,
                                            Scope & lookup_scope,
                                            const string & name)
{
  const string normalized_name = ctx.normalize_type_lookup_name(name);

  const string template_head =
      semantic_utils::strip_trailing_top_level_template_arguments(normalized_name);
  if(template_head != normalized_name && is_simple_identifier_text(template_head)) {
    map<string, AliasTemplateDecl *>::iterator alias_template =
        scope.alias_templates.find(template_head);
    if(alias_template != scope.alias_templates.end()) {
      const Scope * qualification_scope = &scope;
      if(alias_template->second && alias_template->second->declaring_scope) {
        qualification_scope = alias_template->second->declaring_scope;
      } else if(scope.class_info && scope.class_info->enclosing_scope) {
        qualification_scope = scope.class_info->enclosing_scope;
      }
      const string qualified_name =
          scope_qualified_name(*qualification_scope, normalized_name);
      TypePtr type = ctx.lookup_type(lookup_scope, qualified_name, true);
      if(type) {
        return type;
      }
    }

    map<string, ClassTemplateDecl *>::iterator class_template =
        scope.class_templates.find(template_head);
    if(class_template != scope.class_templates.end()) {
      const Scope * qualification_scope = &scope;
      if(class_template->second && class_template->second->declaring_scope) {
        qualification_scope = class_template->second->declaring_scope;
      } else if(scope.class_info && scope.class_info->enclosing_scope) {
        qualification_scope = scope.class_info->enclosing_scope;
      }
      const string qualified_name =
          scope_qualified_name(*qualification_scope, normalized_name);
      TypePtr type = ctx.lookup_type(lookup_scope, qualified_name, true);
      if(type) {
        return type;
      }
    }
  }

  map<string, TypePtr>::const_iterator found = scope.named_types.find(normalized_name);
  if(found != scope.named_types.end()) {
    if(scope.class_info) {
      const MemberAccess direct_access =
          named_type_access_for_lookup(scope, normalized_name);
      if(!member_access_allowed(&lookup_scope,
                                current_class_scope(lookup_scope),
                                current_function_scope(lookup_scope),
                                scope.class_info,
                                direct_access,
                                MA_PUBLIC)) {
        return TypePtr();
      }
    }
    if(scope.class_info &&
       scope.class_info->source_template &&
       !scope.class_info->dependent_instantiation &&
       found->second &&
       ctx.type_depends_on_template_parameter(found->second)) {
      TypePtr resolved;
      if(semantic_dependent_type::resolve_instantiated_dependent_type(ctx, scope, found->second, resolved) &&
         resolved &&
         !ctx.type_depends_on_template_parameter(resolved)) {
        return resolved;
      }
    }
    return found->second;
  }

  if(scope.class_info) {
    const bool ensure_current = !is_self_full_collection_scope(scope, *scope.class_info);
    MemberTypeLookupResult member =
        lookup_member_type(ctx,
                           *scope.class_info,
                           normalized_name,
                           ensure_current,
                           &lookup_scope);
    if(member.type) {
      return member.type;
    }
  }

  return TypePtr();
}

TypePtr resolve_direct_type_qualifier_impl(SemanticContext & ctx,
                                           Scope & scope,
                                           Scope & lookup_scope,
                                           const string & name)
{
  TypePtr direct = resolve_direct_type_qualifier_local(ctx, scope, lookup_scope, name);
  if(direct) {
    return direct;
  }
  return lookup_inline_namespace_children<TypePtr>(
      scope,
      [&ctx, &lookup_scope, &name](Scope & child) -> TypePtr
      {
        return resolve_direct_type_qualifier_local(ctx, child, lookup_scope, name);
      },
      [](const TypePtr & type) -> bool
      {
        return static_cast<bool>(type);
      });
}

TypePtr lookup_type_from_using_directives_in_scope(SemanticContext & ctx,
                                                   Scope & scope,
                                                   Scope & lookup_scope,
                                                   const string & name)
{
  if(scope.using_directives.empty()) {
    return TypePtr();
  }
  bool found = false;
  bool ambiguous = false;
  TypePtr value;
  std::set<const Scope *> visited;
  cpp_scope_lookup::collect_lookup_from_using_directives<TypePtr>(
      scope,
      name,
      visited,
      [&ctx, &lookup_scope](Scope & target, const string & lookup_name) -> TypePtr
      {
        return resolve_direct_type_qualifier_impl(ctx,
                                                  target,
                                                  lookup_scope,
                                                  lookup_name);
      },
      [](const TypePtr & type) -> bool
      {
        return static_cast<bool>(type);
      },
      found,
      value,
      ambiguous);
  return ambiguous ? TypePtr() : value;
}

TypePtr resolve_type_qualifier_in_qualified_scope(SemanticContext & ctx,
                                                  Scope & scope,
                                                  Scope & lookup_scope,
                                                  const string & name)
{
  TypePtr direct = resolve_direct_type_qualifier_impl(ctx, scope, lookup_scope, name);
  TypePtr imported =
      lookup_type_from_using_directives_in_scope(ctx, scope, lookup_scope, name);
  if(direct && imported && !type_equals(direct, imported)) {
    return TypePtr();
  }
  return direct ? direct : imported;
}

TypePtr lookup_qualifier_type_name(SemanticContext & ctx,
                                   Scope & direct_scope,
                                   Scope & lookup_scope,
                                   const string & text,
                                   bool allow_direct_lookup)
{
  const string trimmed = semantic_utils::trim_space(text);
  if(has_top_level_declarator_syntax(trimmed)) {
    return TypePtr();
  }

  TypePtr type;
  if(allow_direct_lookup) {
    type =
        resolve_type_qualifier_in_qualified_scope(ctx, direct_scope, lookup_scope, trimmed);
    if(type) {
      return type;
    }
  }

  type = ctx.lookup_type(lookup_scope, trimmed, true);
  if(type) {
    return type;
  }

  return TypePtr();
}

Scope * lookup_namespace_from_using_directives_in_scope(Scope & scope,
                                                        const string & name)
{
  if(scope.using_directives.empty()) {
    return nullptr;
  }
  bool found = false;
  bool ambiguous = false;
  Scope * value = nullptr;
  std::set<const Scope *> visited;
  collect_decl_lookup_from_using_directives<Scope>(
      scope,
      name,
      visited,
      [](Scope & target, const string & lookup_name) -> Scope *
      {
        return resolve_direct_namespace(target, lookup_name);
      },
      [](Scope * lhs, Scope * rhs) -> bool
      {
        return lhs == rhs;
      },
      found,
      value,
      ambiguous);
  return ambiguous ? nullptr : value;
}

Scope * lookup_namespace_from_scope(Scope & scope, const string & name)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(Scope * direct = resolve_direct_namespace(*current, name)) {
      return direct;
    }
    if(Scope * imported = lookup_namespace_from_using_directives_in_scope(*current, name)) {
      return imported;
    }
  }
  return nullptr;
}

Scope * lookup_namespace_member_in_qualified_scope(Scope & scope,
                                                   const string & name)
{
  Scope * direct = resolve_direct_namespace(scope, name);
  if(direct) {
    return direct;
  }
  return lookup_namespace_from_using_directives_in_scope(scope, name);
}

bool scope_can_form_definition_path(const Scope & scope)
{
  for(const Scope * current = &scope; current; current = current->parent) {
    if((current->namespace_scope || is_named_class_member_scope(current)) &&
       current->name != "<global>" &&
       current->name != "<unnamed>") {
      return true;
    }
  }
  return false;
}

Scope * lookup_namespace_from_definition_path(Scope & scope,
                                              Scope & definition_scope,
                                              const string & name)
{
  if(!scope_can_form_definition_path(definition_scope)) {
    return nullptr;
  }
  const string qualified_text = scope_qualified_name(definition_scope, name);
  QualifiedName qualified;
  if(!semantic_utils::split_qualified_name_text(qualified_text, qualified) ||
     qualified.qualifiers.empty()) {
    return nullptr;
  }
  qualified.rooted = true;
  return lookup_namespace_name(*root_scope(scope), qualified);
}

Scope * lookup_namespace_from_template_source_scopes(Scope & scope,
                                                     const string & name)
{
  array<Scope *, 16> visited = {};
  size_t visited_count = 0;
  const auto mark_visited = [&](Scope * candidate) -> bool
  {
    for(size_t i = 0; i < visited_count; ++i) {
      if(visited[i] == candidate) {
        return false;
      }
    }
    if(visited_count < visited.size()) {
      visited[visited_count++] = candidate;
    }
    return true;
  };
  const auto try_scope = [&](Scope * candidate) -> Scope *
  {
    if(!candidate || candidate == &scope || !mark_visited(candidate)) {
      return nullptr;
    }
    Scope * found = lookup_namespace_from_scope(*candidate, name);
    if(found) {
      return found;
    }
    return lookup_namespace_from_definition_path(scope, *candidate, name);
  };

  for(Scope * current = &scope; current; current = current->parent) {
    if(!current->class_info) {
      continue;
    }
    if(current->class_info->source_template) {
      if(Scope * found =
             try_scope(current->class_info->source_template->declaring_scope)) {
        return found;
      }
      if(Scope * found =
             try_scope(current->class_info->source_template->pattern_scope)) {
        return found;
      }
    }
    if(Scope * found = try_scope(current->class_info->enclosing_scope)) {
      return found;
    }
  }
  return nullptr;
}

Scope * resolve_type_qualifier_scope(SemanticContext & ctx,
                                     Scope & scope,
                                     Scope & lookup_scope,
                                     const string & name,
                                     bool allow_dependent_class_qualifiers)
{
  Scope * as_namespace = lookup_namespace_from_scope(scope, name);
  if(!as_namespace) {
    as_namespace = lookup_namespace_from_template_source_scopes(scope, name);
  }
  if(as_namespace) {
    return as_namespace;
  }

  TypePtr as_type =
      lookup_qualifier_type_name(ctx, scope, lookup_scope, name, true);
  if(as_type &&
     ctx.type_depends_on_template_parameter(as_type) &&
     !allow_dependent_class_qualifiers &&
     !is_enclosing_current_specialization_type(lookup_scope, as_type)) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "defer-dependent-qualifier name=" << name
            << " type=" << describe_type(as_type);
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return nullptr;
  }
  Scope * type_scope = ctx.scope_for_type(as_type);
  if(type_scope) {
    return type_scope;
  }
  ClassInfo * info = ctx.class_info_for_type(as_type);
  if(!info) {
    parser_trace::push_use_location("\x1d");
    info = ctx.complete_class_type(as_type);
    parser_trace::pop_use_location();
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "resolve-qualifier-scope name=" << name
          << " in=" << scope_qualified_name(scope, "<here>")
          << " type=" << (as_type ? describe_type(as_type) : string("<none>"));
    TypePtr base = strip_top_level_cv(as_type);
    if(base && base->kind == Type::TK_NAMED) {
      trace << " key=" << base->named_key;
    }
    trace << " class="
          << (info ? info->qualified_name : string("<none>"));
    if(info) {
      trace << " complete=" << (info->complete ? "yes" : "no")
            << " ref-members=" << (info->reference_members_collected ? "yes" : "no")
            << " source-template=" << (info->source_template ? "yes" : "no")
            << " class-node="
            << (info->class_node ? cppast_kind_text(info->class_node->kind) : string("<none>"))
            << " template-node="
            << (info->template_output_node
                    ? cppast_kind_text(info->template_output_node->kind)
                    : string("<none>"));
    }
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  if(info) {
    const string use_location = parser_trace::current_order_use_location();
    if(!use_location.empty()) {
      info->first_qualifier_use_location =
          prefer_earlier_source_location(info->first_qualifier_use_location,
                                         use_location);
    }
    if(is_concrete_class_template_qualifier(ctx, info) &&
       !info->complete &&
       !info->full_member_collection_in_progress &&
       !info->template_instantiation_in_progress) {
      parser_trace::push_use_location("\x1d");
      if(ClassInfo * completed = ctx.complete_class_type(as_type)) {
        info = completed;
      }
      parser_trace::pop_use_location();
    }
    if(!info->complete) {
      ensure_class_reference_members_if_needed(ctx, scope, *info);
    }
    if(!info->member_scope) {
      parser_trace::push_use_location("\x1d");
      info = ctx.complete_class_type(as_type);
      parser_trace::pop_use_location();
      type_scope = ctx.scope_for_type(as_type);
      if(type_scope) {
        return type_scope;
      }
    }
  }
  return info ? info->member_scope.get() : nullptr;
}

Scope * resolve_nested_type_qualifier_scope(SemanticContext & ctx,
                                            Scope & scope,
                                            Scope & lookup_scope,
                                            const string & name,
                                            bool allow_dependent_class_qualifiers)
{
  Scope * as_namespace = resolve_direct_namespace(scope, name);
  if(as_namespace) {
    return as_namespace;
  }

  TypePtr as_type =
      lookup_qualifier_type_name(ctx, scope, lookup_scope, name, true);
  if(as_type &&
     ctx.type_depends_on_template_parameter(as_type) &&
     !allow_dependent_class_qualifiers &&
     !is_enclosing_current_specialization_type(lookup_scope, as_type)) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "defer-dependent-nested-qualifier name=" << name
            << " type=" << describe_type(as_type);
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return nullptr;
  }
  Scope * type_scope = ctx.scope_for_type(as_type);
  if(type_scope) {
    return type_scope;
  }
  ClassInfo * info = ctx.class_info_for_type(as_type);
  if(!info) {
    parser_trace::push_use_location("\x1d");
    info = ctx.complete_class_type(as_type);
    parser_trace::pop_use_location();
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "resolve-nested-qualifier-scope name=" << name
          << " in=" << scope_qualified_name(scope, "<here>")
          << " type=" << (as_type ? describe_type(as_type) : string("<none>"));
    TypePtr base = strip_top_level_cv(as_type);
    if(base && base->kind == Type::TK_NAMED) {
      trace << " key=" << base->named_key;
    }
    trace << " class="
          << (info ? info->qualified_name : string("<none>"));
    if(info) {
      trace << " complete=" << (info->complete ? "yes" : "no")
            << " ref-members=" << (info->reference_members_collected ? "yes" : "no")
            << " source-template=" << (info->source_template ? "yes" : "no")
            << " class-node="
            << (info->class_node ? cppast_kind_text(info->class_node->kind) : string("<none>"))
            << " template-node="
            << (info->template_output_node
                    ? cppast_kind_text(info->template_output_node->kind)
                    : string("<none>"));
    }
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  if(info) {
    const string use_location = parser_trace::current_order_use_location();
    if(!use_location.empty()) {
      info->first_qualifier_use_location =
          prefer_earlier_source_location(info->first_qualifier_use_location,
                                         use_location);
    }
    if(is_concrete_class_template_qualifier(ctx, info) &&
       !info->complete &&
       !info->full_member_collection_in_progress &&
       !info->template_instantiation_in_progress) {
      parser_trace::push_use_location("\x1d");
      if(ClassInfo * completed = ctx.complete_class_type(as_type)) {
        info = completed;
      }
      parser_trace::pop_use_location();
    }
    if(!info->complete) {
      ensure_class_reference_members_if_needed(ctx, scope, *info);
    }
    if(!info->member_scope) {
      parser_trace::push_use_location("\x1d");
      info = ctx.complete_class_type(as_type);
      parser_trace::pop_use_location();
      type_scope = ctx.scope_for_type(as_type);
      if(type_scope) {
        return type_scope;
      }
    }
  }
  return info ? info->member_scope.get() : nullptr;
}

template<typename Result, typename FinalLookup>
Result lookup_qualified_class_or_namespace_generic(SemanticContext & ctx,
                                                   Scope & scope,
                                                   const QualifiedName & qualified,
                                                   const FinalLookup & final_lookup)
{
  Scope * target =
      resolve_qualified_scope_for_class_or_namespace(ctx, scope, qualified);
  if(!target) {
    return Result();
  }
  return final_lookup(*target, qualified.name);
}

}  // namespace

TypePtr resolve_direct_type_qualifier(SemanticContext & ctx,
                                      Scope & scope,
                                      Scope & lookup_scope,
                                      const string & name)
{
  return resolve_direct_type_qualifier_impl(ctx, scope, lookup_scope, name);
}

bool resolve_qualified_namespace_entity_target(SemanticContext & ctx,
                                               Scope & scope,
                                               const QualifiedName & qualified,
                                               Scope *& out_scope,
                                               string & out_name)
{
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    out_scope = &scope;
    out_name = qualified.name;
    return true;
  }

  if(qualified.rooted && qualified.qualifiers.empty()) {
    out_scope = root_scope(scope);
    out_name = qualified.name;
    return true;
  }

  Scope * target = resolve_qualified_scope_for_class_or_namespace(ctx, scope, qualified);
  if(!target) {
    return false;
  }

  out_scope = target;
  out_name = qualified.name;
  return true;
}

Scope * resolve_qualified_variable_parse_scope(SemanticContext & ctx,
                                               Scope & scope,
                                               const CppAstNode & declarator)
{
  const CppAstNode * identifier = first_identifier_node(declarator);
  const QualifiedName * identifier_name =
      identifier ? cppast_qualified_name_syntax(*identifier) : nullptr;
  if(!identifier_name ||
     (!identifier_name->rooted && identifier_name->qualifiers.empty())) {
    return &scope;
  }

  Scope * declaration_scope = &scope;
  string declaration_name = identifier->value;
  if(!resolve_qualified_namespace_entity_target(ctx,
                                                scope,
                                                *identifier_name,
                                                declaration_scope,
                                                declaration_name)) {
    return &scope;
  }
  return declaration_scope;
}

Scope * resolve_direct_namespace_local(Scope & scope, const string & name)
{
  map<string, Scope *>::iterator it = scope.namespace_bindings.find(name);
  return it == scope.namespace_bindings.end() ? nullptr : it->second;
}

const ValueBinding * lookup_direct_value_local(Scope & scope, const string & name)
{
  map<string, ValueBinding>::const_iterator found = scope.values.find(name);
  return found == scope.values.end() ? nullptr : &found->second;
}

const ValueBinding * lookup_direct_value(Scope & scope, const string & name)
{
  const ValueBinding * direct = lookup_direct_value_local(scope, name);
  if(direct) {
    return direct;
  }
  return lookup_inline_namespace_children<const ValueBinding *>(
      scope,
      [&name](Scope & child) -> const ValueBinding *
      {
        return lookup_direct_value_local(child, name);
      },
      [](const ValueBinding * binding) -> bool
      {
        return binding != nullptr;
      });
}

bool same_value_binding_entity(const ValueBinding * lhs, const ValueBinding * rhs)
{
  if(lhs == rhs) {
    return true;
  }
  if(!lhs || !rhs) {
    return false;
  }
  if(lhs->kind != rhs->kind ||
     lhs->name != rhs->name ||
     !type_equals(lhs->type, rhs->type)) {
    return false;
  }
  if(lhs->declaration_node && rhs->declaration_node) {
    return lhs->declaration_node == rhs->declaration_node;
  }
  if(lhs->declaration_scope && rhs->declaration_scope &&
     lhs->declaration_scope == rhs->declaration_scope) {
    return true;
  }
  if(!lhs->symbol.internal_symbol.empty() || !rhs->symbol.internal_symbol.empty() ||
     !lhs->symbol.object_symbol.empty() || !rhs->symbol.object_symbol.empty()) {
    return lhs->symbol.internal_symbol == rhs->symbol.internal_symbol &&
           lhs->symbol.object_symbol == rhs->symbol.object_symbol &&
           lhs->symbol.linkage == rhs->symbol.linkage;
  }
  return false;
}

Scope * resolve_direct_namespace(Scope & scope, const string & name)
{
  Scope * direct = resolve_direct_namespace_local(scope, name);
  if(direct) {
    return direct;
  }
  return lookup_inline_namespace_children<Scope *>(
      scope,
      [&name](Scope & child) -> Scope *
      {
        return resolve_direct_namespace_local(child, name);
      },
      [](Scope * result) -> bool
      {
        return result != nullptr;
      });
}

string scope_qualified_name(const Scope & scope, const string & name)
{
  vector<string> parts;
  parts.push_back(name);
  for(const Scope * current = &scope; current; current = current->parent) {
    if((current->namespace_scope || is_named_class_member_scope(current)) &&
       current->name != "<global>" &&
       current->name != "<unnamed>") {
      parts.push_back(current->name);
    }
  }

  string out;
  for(size_t i = parts.size(); i > 0; --i) {
    if(!out.empty()) {
      out += "::";
    }
    out += parts[i - 1];
  }
  return out;
}

QualifiedName scope_qualified_name_syntax(const Scope & scope, const string & name)
{
  QualifiedName out;
  out.name = name;
  vector<string> qualifiers;
  for(const Scope * current = &scope; current; current = current->parent) {
    if((current->namespace_scope || is_named_class_member_scope(current)) &&
       current->name != "<global>" &&
       current->name != "<unnamed>") {
      qualifiers.push_back(current->name);
    }
  }
  out.qualifiers.reserve(qualifiers.size());
  for(size_t i = qualifiers.size(); i > 0; --i) {
    out.qualifiers.push_back(qualifiers[i - 1]);
  }
  return out;
}

string scope_symbol_qualified_name(const Scope & scope, const string & name)
{
  vector<string> parts;
  parts.push_back(name);
  for(const Scope * current = &scope; current; current = current->parent) {
    if(current->name == "<global>") {
      continue;
    }
    if(current->namespace_scope) {
      if(current->name == "<unnamed>") {
        parts.push_back("_GLOBAL__N_1");
        continue;
      }
      parts.push_back(current->name);
      continue;
    }
    if(is_named_class_member_scope(current)) {
      parts.push_back(current->name);
    }
  }

  string out;
  for(size_t i = parts.size(); i > 0; --i) {
    if(!out.empty()) {
      out += "::";
    }
    out += parts[i - 1];
  }
  return out;
}

QualifiedName scope_symbol_qualified_name_syntax(const Scope & scope, const string & name)
{
  QualifiedName out;
  out.name = name;
  vector<string> qualifiers;
  for(const Scope * current = &scope; current; current = current->parent) {
    if(current->name == "<global>") {
      continue;
    }
    if(current->namespace_scope) {
      qualifiers.push_back(current->name == "<unnamed>" ? string("_GLOBAL__N_1") :
                                                          current->name);
      continue;
    }
    if(is_named_class_member_scope(current)) {
      qualifiers.push_back(current->name);
    }
  }
  out.qualifiers.reserve(qualifiers.size());
  for(size_t i = qualifiers.size(); i > 0; --i) {
    out.qualifiers.push_back(qualifiers[i - 1]);
  }
  return out;
}

string inline_namespace_collapsed_scope_name(const Scope * scope)
{
  if(!scope) {
    return string();
  }

  vector<string> parts;
  for(const Scope * current = scope; current; current = current->parent) {
    if(current->parent == nullptr) {
      break;
    }
    if(!current->name.empty() && !current->inline_namespace) {
      parts.push_back(current->name);
    }
  }

  string out;
  for(size_t i = parts.size(); i > 0; --i) {
    if(!out.empty()) {
      out += "::";
    }
    out += parts[i - 1];
  }
  return out;
}

bool same_inline_namespace_function_template_entity(const FunctionTemplateDecl * lhs,
                                                    const FunctionTemplateDecl * rhs);

bool same_inline_namespace_function_entity(const FunctionBinding & lhs,
                                           const FunctionBinding & rhs)
{
  const bool have_symbol_identity =
      (!lhs.symbol.internal_symbol.empty() || !lhs.symbol.object_symbol.empty() ||
       !rhs.symbol.internal_symbol.empty() || !rhs.symbol.object_symbol.empty());
  if(have_symbol_identity &&
     lhs.symbol.internal_symbol == rhs.symbol.internal_symbol &&
     lhs.symbol.object_symbol == rhs.symbol.object_symbol &&
     lhs.symbol.linkage == rhs.symbol.linkage &&
     lhs.name == rhs.name &&
     cpp_decl::type_equals(lhs.type, rhs.type)) {
    return true;
  }
  if(lhs.declaration_node && rhs.declaration_node &&
     lhs.declaration_node == rhs.declaration_node &&
     lhs.name == rhs.name &&
     cpp_decl::type_equals(lhs.type, rhs.type)) {
    return true;
  }
  if(lhs.declaration_scope && rhs.declaration_scope &&
     lhs.declaration_scope == rhs.declaration_scope &&
     lhs.name == rhs.name &&
     cpp_decl::type_equals(lhs.type, rhs.type) &&
     lhs.owner_class == rhs.owner_class &&
     !lhs.source_template &&
     !rhs.source_template) {
    return true;
  }
  if(lhs.source_template || rhs.source_template) {
    return lhs.source_template &&
           rhs.source_template &&
           !lhs.owner_class &&
           !rhs.owner_class &&
           lhs.template_instantiation_key == rhs.template_instantiation_key &&
           same_effective_function_entity_name(lhs, rhs) &&
           cpp_decl::type_equals(lhs.type, rhs.type) &&
           same_inline_namespace_function_template_entity(lhs.source_template,
                                                         rhs.source_template);
  }
  return !lhs.owner_class &&
         !rhs.owner_class &&
         same_effective_function_entity_name(lhs, rhs) &&
         cpp_decl::type_equals(lhs.type, rhs.type);
}

string canonical_function_lookup_name(const string & name)
{
  if(!function_lookup_name_needs_canonicalization(name)) {
    return name;
  }

  const size_t scope_pos = semantic_utils::top_level_scope_split(name);
  const string prefix = scope_pos == string::npos ? string() : name.substr(0, scope_pos + 2);
  const string unqualified = scope_pos == string::npos ? name : name.substr(scope_pos + 2);
  if(unqualified.compare(0, 8, "operator") != 0) {
    return name;
  }

  string suffix = unqualified.substr(8);
  if(suffix.empty()) {
    return name;
  }

  const size_t first_non_space = suffix.find_first_not_of(' ');
  if(first_non_space == string::npos) {
    return prefix + "operator";
  }
  if(first_non_space != 0) {
    suffix = suffix.substr(first_non_space);
  }
  return prefix + "operator" + suffix;
}

bool template_parameter_index_for_type(const std::vector<TemplateParameterInfo> & parameters,
                                       const TypePtr & type,
                                       std::size_t & index)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return false;
  }
  const TemplateParameterInfo * parameter =
      find_template_parameter(
          parameters,
          strip_leading_typename_token(strip_friend_type_prefixes(base->named_key)));
  if(!parameter && base->named_display != base->named_key) {
    parameter =
        find_template_parameter(
            parameters,
            strip_leading_typename_token(strip_friend_type_prefixes(base->named_display)));
  }
  if(!parameter) {
    return false;
  }
  index = static_cast<std::size_t>(parameter - &parameters[0]);
  return true;
}

std::string canonicalize_template_parameter_tokens(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::string & text)
{
  if(text.empty()) {
    return text;
  }
  std::map<std::string, std::string> replacements;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].name.empty()) {
      continue;
    }
    replacements[parameters[i].name] = std::string("__cppgm_tparam_") + std::to_string(i);
  }
  if(replacements.empty()) {
    return text;
  }

  std::string out;
  out.reserve(text.size());
  for(std::size_t i = 0; i < text.size();) {
    if(is_identifier_char(text[i])) {
      const std::size_t start = i;
      while(i < text.size() && is_identifier_char(text[i])) {
        ++i;
      }
      const std::string token = text.substr(start, i - start);
      std::map<std::string, std::string>::const_iterator found = replacements.find(token);
      out += found == replacements.end() ? token : found->second;
      continue;
    }
    out += text[i++];
  }
  return out;
}

std::string canonicalize_template_named_type_text(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::string & text)
{
  return strip_leading_typename_token(
      strip_friend_type_prefixes(
          canonicalize_template_parameter_tokens(parameters, text)));
}

bool same_function_template_entity_type_impl(
    const TypePtr & lhs_type,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const TypePtr & rhs_type,
    const std::vector<TemplateParameterInfo> & rhs_parameters,
    bool ignore_top_level_cv);

std::string canonicalize_dependent_template_argument_text(
    const std::vector<TemplateParameterInfo> & parameters,
    const DependentAliasTemplateArgumentSyntax & argument)
{
  const std::string text =
      !argument.text.empty() ? argument.text : argument.syntax.text;
  return canonicalize_template_named_type_text(
      parameters,
      strip_leading_typename_token(text));
}

bool same_dependent_template_argument_metadata(
    const DependentAliasTemplateArgumentSyntax & lhs,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const DependentAliasTemplateArgumentSyntax & rhs,
    const std::vector<TemplateParameterInfo> & rhs_parameters)
{
  if(lhs.syntax.pack_expansion != rhs.syntax.pack_expansion) {
    return false;
  }
  if(lhs.type || rhs.type) {
    return lhs.type &&
           rhs.type &&
           same_function_template_entity_type_impl(lhs.type,
                                                   lhs_parameters,
                                                   rhs.type,
                                                   rhs_parameters,
                                                   true);
  }
  return canonicalize_dependent_template_argument_text(lhs_parameters, lhs) ==
         canonicalize_dependent_template_argument_text(rhs_parameters, rhs);
}

bool same_dependent_template_argument_metadata_list(
    const std::vector<DependentAliasTemplateArgumentSyntax> & lhs,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const std::vector<DependentAliasTemplateArgumentSyntax> & rhs,
    const std::vector<TemplateParameterInfo> & rhs_parameters)
{
  if(lhs.size() != rhs.size()) {
    return false;
  }
  for(std::size_t i = 0; i < lhs.size(); ++i) {
    if(!same_dependent_template_argument_metadata(lhs[i],
                                                  lhs_parameters,
                                                  rhs[i],
                                                  rhs_parameters)) {
      return false;
    }
  }
  return true;
}

bool same_dependent_qualified_member_template_ids(
    const std::vector<TemplateIdSyntax> & lhs,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const std::vector<TemplateIdSyntax> & rhs,
    const std::vector<TemplateParameterInfo> & rhs_parameters)
{
  if(lhs.size() != rhs.size()) {
    return false;
  }
  for(std::size_t i = 0; i < lhs.size(); ++i) {
    const std::string lhs_name =
        canonicalize_template_named_type_text(
            lhs_parameters,
            qualified_name_text(lhs[i].name));
    const std::string rhs_name =
        canonicalize_template_named_type_text(
            rhs_parameters,
            qualified_name_text(rhs[i].name));
    if(lhs_name != rhs_name ||
       lhs[i].argument_syntaxes.size() != rhs[i].argument_syntaxes.size()) {
      return false;
    }
    for(std::size_t arg = 0; arg < lhs[i].argument_syntaxes.size(); ++arg) {
      const std::string lhs_argument_text =
          !lhs[i].argument_syntaxes[arg].text.empty() ?
              lhs[i].argument_syntaxes[arg].text :
              (arg < lhs[i].arguments.size() ? lhs[i].arguments[arg] : std::string());
      const std::string rhs_argument_text =
          !rhs[i].argument_syntaxes[arg].text.empty() ?
              rhs[i].argument_syntaxes[arg].text :
              (arg < rhs[i].arguments.size() ? rhs[i].arguments[arg] : std::string());
      const std::string lhs_text =
          canonicalize_template_named_type_text(
              lhs_parameters,
              strip_leading_typename_token(lhs_argument_text));
      const std::string rhs_text =
          canonicalize_template_named_type_text(
              rhs_parameters,
              strip_leading_typename_token(rhs_argument_text));
      if(lhs_text != rhs_text) {
        return false;
      }
    }
  }
  return true;
}

bool same_named_dependent_qualified_member_metadata(
    const TypePtr & lhs_type,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const TypePtr & rhs_type,
    const std::vector<TemplateParameterInfo> & rhs_parameters,
    bool & metadata_present)
{
  metadata_present = false;
  TypePtr lhs_owner;
  TypePtr rhs_owner;
  std::vector<std::string> lhs_members;
  std::vector<std::string> rhs_members;
  std::vector<TemplateIdSyntax> lhs_template_ids;
  std::vector<TemplateIdSyntax> rhs_template_ids;
  bool lhs_leading_typename = false;
  bool rhs_leading_typename = false;
  const bool lhs_dependent_member =
      named_type_dependent_qualified_member(lhs_type,
                                            lhs_owner,
                                            lhs_members,
                                            lhs_leading_typename,
                                            &lhs_template_ids);
  const bool rhs_dependent_member =
      named_type_dependent_qualified_member(rhs_type,
                                            rhs_owner,
                                            rhs_members,
                                            rhs_leading_typename,
                                            &rhs_template_ids);
  if(lhs_dependent_member || rhs_dependent_member) {
    metadata_present = true;
    return lhs_dependent_member &&
           rhs_dependent_member &&
           lhs_members == rhs_members &&
           same_dependent_qualified_member_template_ids(lhs_template_ids,
                                                        lhs_parameters,
                                                        rhs_template_ids,
                                                        rhs_parameters) &&
           same_function_template_entity_type_impl(lhs_owner,
                                                   lhs_parameters,
                                                   rhs_owner,
                                                   rhs_parameters,
                                                   true);
  }
  return false;
}

bool same_named_dependent_template_metadata(
    const TypePtr & lhs_type,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const TypePtr & rhs_type,
    const std::vector<TemplateParameterInfo> & rhs_parameters,
    bool & metadata_present)
{
  metadata_present = false;
  TypePtr lhs_base = strip_top_level_cv(lhs_type);
  TypePtr rhs_base = strip_top_level_cv(rhs_type);
  if(!lhs_base ||
     !rhs_base ||
     lhs_base->kind != Type::TK_NAMED ||
     rhs_base->kind != Type::TK_NAMED) {
    return false;
  }

  void * lhs_decl = nullptr;
  void * rhs_decl = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> lhs_args;
  std::vector<DependentAliasTemplateArgumentSyntax> rhs_args;
  const bool lhs_class =
      named_type_dependent_class_template(lhs_base, lhs_decl, lhs_args);
  const bool rhs_class =
      named_type_dependent_class_template(rhs_base, rhs_decl, rhs_args);
  if(lhs_class || rhs_class) {
    metadata_present = true;
    return lhs_class &&
           rhs_class &&
           lhs_decl == rhs_decl &&
           same_dependent_template_argument_metadata_list(lhs_args,
                                                          lhs_parameters,
                                                          rhs_args,
                                                          rhs_parameters);
  }

  const bool lhs_alias =
      named_type_dependent_alias_template(lhs_base, lhs_decl, lhs_args);
  const bool rhs_alias =
      named_type_dependent_alias_template(rhs_base, rhs_decl, rhs_args);
  if(lhs_alias || rhs_alias) {
    metadata_present = true;
    return lhs_alias &&
           rhs_alias &&
           lhs_decl == rhs_decl &&
           same_dependent_template_argument_metadata_list(lhs_args,
                                                          lhs_parameters,
                                                          rhs_args,
                                                          rhs_parameters);
  }

  return false;
}

bool same_inline_namespace_template_parameter_type(
    const TypePtr & lhs_type,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const TypePtr & rhs_type,
    const std::vector<TemplateParameterInfo> & rhs_parameters)
{
  if(!lhs_type || !rhs_type) {
    return lhs_type == rhs_type;
  }
  if(type_equals(lhs_type, rhs_type)) {
    return true;
  }

  std::size_t lhs_parameter_index = 0;
  std::size_t rhs_parameter_index = 0;
  const bool lhs_is_parameter =
      template_parameter_index_for_type(lhs_parameters, lhs_type, lhs_parameter_index);
  const bool rhs_is_parameter =
      template_parameter_index_for_type(rhs_parameters, rhs_type, rhs_parameter_index);
  if(lhs_is_parameter || rhs_is_parameter) {
    if(!lhs_is_parameter || !rhs_is_parameter ||
       lhs_parameter_index != rhs_parameter_index) {
      return false;
    }
    const TemplateParameterInfo & lhs_parameter = lhs_parameters[lhs_parameter_index];
    const TemplateParameterInfo & rhs_parameter = rhs_parameters[rhs_parameter_index];
    return lhs_parameter.kind == rhs_parameter.kind &&
           lhs_parameter.parameter_pack == rhs_parameter.parameter_pack &&
           lhs_parameter.template_parameter_count == rhs_parameter.template_parameter_count;
  }

  if(lhs_type->kind != rhs_type->kind) {
    return false;
  }

  switch(lhs_type->kind) {
  case Type::TK_FUNDAMENTAL:
    return lhs_type->fundamental == rhs_type->fundamental;

  case Type::TK_NAMED:
  {
    bool metadata_present = false;
    if(same_named_dependent_qualified_member_metadata(lhs_type,
                                                      lhs_parameters,
                                                      rhs_type,
                                                      rhs_parameters,
                                                      metadata_present)) {
      return true;
    }
    if(metadata_present) {
      return false;
    }
    if(same_named_dependent_template_metadata(lhs_type,
                                              lhs_parameters,
                                              rhs_type,
                                              rhs_parameters,
                                              metadata_present)) {
      return true;
    }
    if(metadata_present) {
      return false;
    }
    const std::string lhs_key =
        canonicalize_template_named_type_text(lhs_parameters, lhs_type->named_key);
    const std::string rhs_key =
        canonicalize_template_named_type_text(rhs_parameters, rhs_type->named_key);
    const std::string lhs_display =
        canonicalize_template_named_type_text(lhs_parameters, lhs_type->named_display);
    const std::string rhs_display =
        canonicalize_template_named_type_text(rhs_parameters, rhs_type->named_display);
    const std::string lhs_key_normalized =
        semantic_utils::trim_space(semantic_utils::strip_elaborated_type_prefix(lhs_key));
    const std::string rhs_key_normalized =
        semantic_utils::trim_space(semantic_utils::strip_elaborated_type_prefix(rhs_key));
    const bool lhs_qualified =
        semantic_utils::top_level_scope_split(lhs_key_normalized) != std::string::npos;
    const bool rhs_qualified =
        semantic_utils::top_level_scope_split(rhs_key_normalized) != std::string::npos;
    if(lhs_qualified && rhs_qualified && lhs_key_normalized != rhs_key_normalized) {
      return false;
    }
    return lhs_key == rhs_key || lhs_key == rhs_display ||
           lhs_display == rhs_key || lhs_display == rhs_display;
  }

  case Type::TK_CV:
    return lhs_type->cv_const == rhs_type->cv_const &&
           lhs_type->cv_volatile == rhs_type->cv_volatile &&
           same_inline_namespace_template_parameter_type(lhs_type->inner,
                                                         lhs_parameters,
                                                         rhs_type->inner,
                                                         rhs_parameters);

  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return same_inline_namespace_template_parameter_type(lhs_type->inner,
                                                         lhs_parameters,
                                                         rhs_type->inner,
                                                         rhs_parameters);

  case Type::TK_MEMBER_POINTER:
    return same_inline_namespace_template_parameter_type(lhs_type->owner,
                                                         lhs_parameters,
                                                         rhs_type->owner,
                                                         rhs_parameters) &&
           same_inline_namespace_template_parameter_type(lhs_type->inner,
                                                         lhs_parameters,
                                                         rhs_type->inner,
                                                         rhs_parameters);

  case Type::TK_ARRAY:
    return lhs_type->has_bound == rhs_type->has_bound &&
           lhs_type->bound == rhs_type->bound &&
           lhs_type->bound_text == rhs_type->bound_text &&
           same_inline_namespace_template_parameter_type(lhs_type->inner,
                                                         lhs_parameters,
                                                         rhs_type->inner,
                                                         rhs_parameters);

  case Type::TK_FUNCTION:
    if(lhs_type->variadic != rhs_type->variadic ||
       lhs_type->prototype_relaxed != rhs_type->prototype_relaxed ||
       lhs_type->function_const != rhs_type->function_const ||
       lhs_type->function_volatile != rhs_type->function_volatile ||
       lhs_type->params.size() != rhs_type->params.size() ||
       !same_inline_namespace_template_parameter_type(lhs_type->inner,
                                                      lhs_parameters,
                                                      rhs_type->inner,
                                                      rhs_parameters)) {
      return false;
    }
    for(std::size_t i = 0; i < lhs_type->params.size(); ++i) {
      if(!same_inline_namespace_template_parameter_type(lhs_type->params[i],
                                                        lhs_parameters,
                                                        rhs_type->params[i],
                                                        rhs_parameters)) {
        return false;
      }
    }
    return true;
  default:
    return false;
  }
}

bool same_function_template_entity_type_impl(
    const TypePtr & lhs_type,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const TypePtr & rhs_type,
    const std::vector<TemplateParameterInfo> & rhs_parameters,
    bool ignore_top_level_cv)
{
  TypePtr lhs = ignore_top_level_cv ? strip_top_level_cv(lhs_type) : lhs_type;
  TypePtr rhs = ignore_top_level_cv ? strip_top_level_cv(rhs_type) : rhs_type;
  if(!lhs || !rhs) {
    return lhs == rhs;
  }
  if(type_equals(lhs, rhs)) {
    return true;
  }

  auto template_parameter_index =
      [&](const std::vector<TemplateParameterInfo> & parameters,
          const TypePtr & type,
          std::size_t & index) -> bool
  {
    TypePtr base = ignore_top_level_cv ? strip_top_level_cv(type) : type;
    if(!base || base->kind != Type::TK_NAMED) {
      return false;
    }
    const TemplateParameterInfo * parameter =
        find_template_parameter(
            parameters,
            strip_leading_typename_token(strip_friend_type_prefixes(base->named_key)));
    if(!parameter && base->named_display != base->named_key) {
      parameter =
          find_template_parameter(
              parameters,
              strip_leading_typename_token(strip_friend_type_prefixes(base->named_display)));
    }
    if(!parameter) {
      return false;
    }
    index = static_cast<std::size_t>(parameter - &parameters[0]);
    return true;
  };

  std::size_t lhs_parameter_index = 0;
  std::size_t rhs_parameter_index = 0;
  const bool lhs_is_parameter =
      template_parameter_index(lhs_parameters, lhs, lhs_parameter_index);
  const bool rhs_is_parameter =
      template_parameter_index(rhs_parameters, rhs, rhs_parameter_index);
  if(lhs_is_parameter || rhs_is_parameter) {
    if(!lhs_is_parameter || !rhs_is_parameter ||
       lhs_parameter_index != rhs_parameter_index) {
      return false;
    }
    const TemplateParameterInfo & lhs_parameter = lhs_parameters[lhs_parameter_index];
    const TemplateParameterInfo & rhs_parameter = rhs_parameters[rhs_parameter_index];
    return lhs_parameter.kind == rhs_parameter.kind &&
           lhs_parameter.parameter_pack == rhs_parameter.parameter_pack &&
           lhs_parameter.template_parameter_count == rhs_parameter.template_parameter_count;
  }

  if(lhs->kind != rhs->kind) {
    return false;
  }

  switch(lhs->kind) {
  case Type::TK_FUNDAMENTAL:
    return lhs->fundamental == rhs->fundamental;

  case Type::TK_NAMED:
  {
    bool metadata_present = false;
    if(same_named_dependent_qualified_member_metadata(lhs,
                                                      lhs_parameters,
                                                      rhs,
                                                      rhs_parameters,
                                                      metadata_present)) {
      return true;
    }
    if(metadata_present) {
      return false;
    }
    if(same_named_dependent_template_metadata(lhs,
                                              lhs_parameters,
                                              rhs,
                                              rhs_parameters,
                                              metadata_present)) {
      return true;
    }
    if(metadata_present) {
      return false;
    }
    const std::string lhs_key =
        canonicalize_template_named_type_text(lhs_parameters, lhs->named_key);
    const std::string rhs_key =
        canonicalize_template_named_type_text(rhs_parameters, rhs->named_key);
    const std::string lhs_display =
        canonicalize_template_named_type_text(lhs_parameters, lhs->named_display);
    const std::string rhs_display =
        canonicalize_template_named_type_text(rhs_parameters, rhs->named_display);
    const std::string lhs_key_normalized =
        semantic_utils::trim_space(semantic_utils::strip_elaborated_type_prefix(lhs_key));
    const std::string rhs_key_normalized =
        semantic_utils::trim_space(semantic_utils::strip_elaborated_type_prefix(rhs_key));
    const bool lhs_qualified =
        semantic_utils::top_level_scope_split(lhs_key_normalized) != std::string::npos;
    const bool rhs_qualified =
        semantic_utils::top_level_scope_split(rhs_key_normalized) != std::string::npos;
    if(lhs_qualified && rhs_qualified && lhs_key_normalized != rhs_key_normalized) {
      return false;
    }
    return lhs_key == rhs_key || lhs_key == rhs_display ||
           lhs_display == rhs_key || lhs_display == rhs_display;
  }

  case Type::TK_CV:
    return lhs->cv_const == rhs->cv_const &&
           lhs->cv_volatile == rhs->cv_volatile &&
           same_function_template_entity_type_impl(lhs->inner,
                                                   lhs_parameters,
                                                   rhs->inner,
                                                   rhs_parameters,
                                                   false);

  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return same_function_template_entity_type_impl(lhs->inner,
                                                   lhs_parameters,
                                                   rhs->inner,
                                                   rhs_parameters,
                                                   false);

  case Type::TK_MEMBER_POINTER:
    return same_function_template_entity_type_impl(lhs->owner,
                                                   lhs_parameters,
                                                   rhs->owner,
                                                   rhs_parameters,
                                                   false) &&
           same_function_template_entity_type_impl(lhs->inner,
                                                   lhs_parameters,
                                                   rhs->inner,
                                                   rhs_parameters,
                                                   false);

  case Type::TK_ARRAY:
    return lhs->has_bound == rhs->has_bound &&
           lhs->bound == rhs->bound &&
           lhs->bound_text == rhs->bound_text &&
           same_function_template_entity_type_impl(lhs->inner,
                                                   lhs_parameters,
                                                   rhs->inner,
                                                   rhs_parameters,
                                                   false);

  case Type::TK_FUNCTION:
    if(lhs->variadic != rhs->variadic ||
       lhs->prototype_relaxed != rhs->prototype_relaxed ||
       lhs->function_const != rhs->function_const ||
       lhs->function_volatile != rhs->function_volatile ||
       lhs->params.size() != rhs->params.size() ||
       !same_function_template_entity_type_impl(lhs->inner,
                                                lhs_parameters,
                                                rhs->inner,
                                                rhs_parameters,
                                                true)) {
      return false;
    }
    for(std::size_t i = 0; i < lhs->params.size(); ++i) {
      if(!same_function_template_entity_type_impl(lhs->params[i],
                                                  lhs_parameters,
                                                  rhs->params[i],
                                                  rhs_parameters,
                                                  true)) {
        return false;
      }
    }
    return true;
  default:
    return false;
  }
}

bool same_function_template_entity_type(
    const TypePtr & lhs_type,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const TypePtr & rhs_type,
    const std::vector<TemplateParameterInfo> & rhs_parameters)
{
  return same_function_template_entity_type_impl(lhs_type,
                                                 lhs_parameters,
                                                 rhs_type,
                                                 rhs_parameters,
                                                 false);
}

struct SameFunctionTemplateEntityCacheEntry
{
  const FunctionTemplateDecl * lhs = nullptr;
  const FunctionTemplateDecl * rhs = nullptr;
  const Type * lhs_type = nullptr;
  const Type * rhs_type = nullptr;
  std::size_t lhs_parameter_count = 0;
  std::size_t rhs_parameter_count = 0;
  std::string lhs_name;
  std::string rhs_name;
  bool result = false;
};

std::size_t same_function_template_entity_cache_hash(const FunctionTemplateDecl * lhs,
                                                     const FunctionTemplateDecl * rhs)
{
  std::size_t seed = reinterpret_cast<std::uintptr_t>(lhs) >> 4;
  seed ^= (reinterpret_cast<std::uintptr_t>(rhs) >> 4) + 0x9e3779b97f4a7c15ULL +
          (seed << 6) + (seed >> 2);
  return seed;
}

bool same_function_template_entity_cache_matches(
    const SameFunctionTemplateEntityCacheEntry & entry,
    const FunctionTemplateDecl * lhs,
    const FunctionTemplateDecl * rhs)
{
  return entry.lhs == lhs &&
         entry.rhs == rhs &&
         entry.lhs_type == lhs->type_pattern.get() &&
         entry.rhs_type == rhs->type_pattern.get() &&
         entry.lhs_parameter_count == lhs->parameters.size() &&
         entry.rhs_parameter_count == rhs->parameters.size() &&
         entry.lhs_name == lhs->name &&
         entry.rhs_name == rhs->name;
}

void store_same_function_template_entity_cache(
    SameFunctionTemplateEntityCacheEntry & entry,
    const FunctionTemplateDecl * lhs,
    const FunctionTemplateDecl * rhs,
    bool result)
{
  entry.lhs = lhs;
  entry.rhs = rhs;
  entry.lhs_type = lhs->type_pattern.get();
  entry.rhs_type = rhs->type_pattern.get();
  entry.lhs_parameter_count = lhs->parameters.size();
  entry.rhs_parameter_count = rhs->parameters.size();
  entry.lhs_name = lhs->name;
  entry.rhs_name = rhs->name;
  entry.result = result;
}

bool same_inline_namespace_function_template_entity(const FunctionTemplateDecl * lhs,
                                                    const FunctionTemplateDecl * rhs)
{
  const auto effective_entity_scope =
      [](const FunctionTemplateDecl * decl) -> const Scope *
  {
    if(!decl || !decl->declaring_scope) {
      return nullptr;
    }
    if(decl->declaring_scope->class_info &&
       !decl->friend_access_classes.empty() &&
       decl->declaring_scope->parent) {
      const ClassInfo * owner = decl->declaring_scope->class_info;
      const bool friend_declared_in_owner =
          std::find(decl->friend_access_classes.begin(),
                    decl->friend_access_classes.end(),
                    owner) != decl->friend_access_classes.end();
      if(friend_declared_in_owner) {
        return decl->declaring_scope->parent;
      }
    }
    return decl->declaring_scope;
  };
  const auto function_type_qualifiers_match =
      [](const FunctionTemplateDecl * lhs_decl, const FunctionTemplateDecl * rhs_decl) -> bool
  {
    TypePtr lhs_type = strip_top_level_cv(lhs_decl->type_pattern);
    TypePtr rhs_type = strip_top_level_cv(rhs_decl->type_pattern);
    if(!lhs_type || !rhs_type ||
       lhs_type->kind != Type::TK_FUNCTION ||
       rhs_type->kind != Type::TK_FUNCTION) {
      return lhs_decl->type_pattern == rhs_decl->type_pattern;
    }
    return lhs_type->variadic == rhs_type->variadic &&
           lhs_type->prototype_relaxed == rhs_type->prototype_relaxed &&
           lhs_type->function_const == rhs_type->function_const &&
           lhs_type->function_volatile == rhs_type->function_volatile;
  };
  if(lhs == rhs) {
    return true;
  }
  if(!lhs || !rhs) {
    return false;
  }
  if(rhs < lhs) {
    std::swap(lhs, rhs);
  }
  static thread_local std::array<SameFunctionTemplateEntityCacheEntry, 4096> cache;
  SameFunctionTemplateEntityCacheEntry & cache_entry =
      cache[same_function_template_entity_cache_hash(lhs, rhs) % cache.size()];
  if(same_function_template_entity_cache_matches(cache_entry, lhs, rhs)) {
    return cache_entry.result;
  }
  const auto store_result =
      [&](bool result) -> bool
      {
        store_same_function_template_entity_cache(cache_entry, lhs, rhs, result);
        return result;
      };
  if(!function_lookup_name_equal(lhs->name, rhs->name) ||
     lhs->is_constructor != rhs->is_constructor ||
     lhs->is_destructor != rhs->is_destructor ||
     lhs->is_static_member != rhs->is_static_member ||
     lhs->is_const_method != rhs->is_const_method ||
     lhs->is_volatile_method != rhs->is_volatile_method ||
     lhs->ref_qualifier != rhs->ref_qualifier) {
    return store_result(false);
  }
  if(!same_inline_namespace_collapsed_scope_identity(effective_entity_scope(lhs),
                                                    effective_entity_scope(rhs))) {
    return store_result(false);
  }
  if(lhs->parameters.size() != rhs->parameters.size() ||
     !function_type_qualifiers_match(lhs, rhs)) {
    return store_result(false);
  }
  if(!same_inline_namespace_template_parameter_list(lhs->parameters,
                                                    rhs->parameters)) {
    return store_result(false);
  }
  return store_result(same_function_template_entity_type(lhs->type_pattern,
                                                        lhs->parameters,
                                                        rhs->type_pattern,
                                                        rhs->parameters));
}

vector<FunctionBinding *> & direct_function_set_slot(Scope & scope, const string & name)
{
  scope.cached_direct_function_lookups.clear();
  ++scope.direct_function_lookup_cache_epoch;
  return scope.function_sets[canonical_function_lookup_name(name)];
}

void set_direct_function_access_override(Scope & scope,
                                         const string & name,
                                         const FunctionBinding * binding,
                                         MemberAccess access)
{
  if(!binding) {
    return;
  }
  scope.function_set_access_overrides[canonical_function_lookup_name(name)][binding] = access;
}

MemberAccess effective_direct_function_access(const Scope & scope,
                                              const string & name,
                                              const FunctionBinding & binding)
{
  const string canonical_name = canonical_function_lookup_name(name);
  map<string, map<const FunctionBinding *, MemberAccess> >::const_iterator found_name =
      scope.function_set_access_overrides.find(canonical_name);
  if(found_name == scope.function_set_access_overrides.end()) {
    return binding.access;
  }
  map<const FunctionBinding *, MemberAccess>::const_iterator found_binding =
      found_name->second.find(&binding);
  return found_binding == found_name->second.end() ? binding.access : found_binding->second;
}

vector<FunctionTemplateDecl *> & direct_function_template_slot(Scope & scope,
                                                               const string & name)
{
  return scope.function_templates[canonical_function_lookup_name(name)];
}

const vector<FunctionBinding *> * find_direct_function_set(const Scope & scope,
                                                           const string & name)
{
  map<string, vector<FunctionBinding *> >::const_iterator found;
  if(function_lookup_name_needs_canonicalization(name)) {
    found = scope.function_sets.find(canonical_function_lookup_name(name));
  } else {
    found = scope.function_sets.find(name);
  }
  return found == scope.function_sets.end() ? nullptr : &found->second;
}

const vector<FunctionTemplateDecl *> * find_direct_function_template_set(const Scope & scope,
                                                                         const string & name)
{
  map<string, vector<FunctionTemplateDecl *> >::const_iterator found =
      scope.function_templates.find(canonical_function_lookup_name(name));
  return found == scope.function_templates.end() ? nullptr : &found->second;
}

FunctionBinding * current_function_scope(Scope & scope)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->function) {
      return current->function;
    }
  }
  return nullptr;
}

ClassInfo * current_class_scope(Scope & scope)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->class_info) {
      return current->class_info;
    }
  }
  return nullptr;
}

bool member_access_allowed(const Scope * lexical_scope,
                           const ClassInfo * current_class,
                           const FunctionBinding * current_function,
                           const ClassInfo * declared_in,
                           MemberAccess member_access,
                           MemberAccess path_access);

bool is_named_enum_type(SemanticContext & ctx, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(base && base->kind == Type::TK_NAMED &&
     base->named_key.compare(0, 5, "enum ") == 0) {
    return true;
  }
  ClassInfo * info = ctx.class_info_for_type(type);
  return info && info->class_kind == "enum";
}

MemberAccess combine_member_access(MemberAccess inherited, MemberAccess edge)
{
  if(inherited == MA_PRIVATE || edge == MA_PRIVATE) {
    return MA_PRIVATE;
  }
  if(inherited == MA_PROTECTED || edge == MA_PROTECTED) {
    return MA_PROTECTED;
  }
  return MA_PUBLIC;
}

bool is_same_or_derived(const ClassInfo * current, const ClassInfo * target)
{
  size_t offset = 0;
  MemberAccess access = MA_PUBLIC;
  return current && target && find_unique_base_path(*current, target, offset, access);
}

bool find_unique_base_path(const ClassInfo & current,
                           const ClassInfo * target,
                           size_t & out_offset,
                           MemberAccess & out_access)
{
  bool found = false;
  size_t matches =
      collect_base_paths(current, target, 0, MA_PUBLIC,
                         [&found, &out_offset, &out_access](size_t offset, MemberAccess access)
                         {
                           if(!found) {
                             found = true;
                             out_offset = offset;
                             out_access = access;
                           }
                         });
  if(matches == 1 && found) {
    return true;
  }
  if(matches > 1) {
    ostringstream out;
    out << "ambiguous base class path";
    out << " [current " << current.qualified_name << "]";
    out << " [target " << (target ? target->qualified_name : string("<null>")) << "]";
    throw logic_error(out.str());
  }
  return false;
}

MemberValueLookupResult lookup_member_value(ClassInfo & info, const string & name)
{
  return lookup_member_in_hierarchy<MemberValueLookupResult>(
      info,
      [&name](ClassInfo & current) -> MemberValueLookupResult
      {
        map<string, ValueBinding>::const_iterator found =
            current.member_scope->values.find(name);
        if(found == current.member_scope->values.end()) {
          return MemberValueLookupResult();
        }
        MemberValueLookupResult result;
        result.binding = &found->second;
        result.declared_in = &current;
        return result;
      });
}

MemberFunctionLookupResult lookup_member_functions(ClassInfo & info, const string & name)
{
  return lookup_member_in_hierarchy<MemberFunctionLookupResult>(
      info,
      [&name](ClassInfo & current) -> MemberFunctionLookupResult
      {
        map<string, vector<FunctionBinding *> >::iterator found =
            current.methods.find(name);
        if(found == current.methods.end() || found->second.empty()) {
          return MemberFunctionLookupResult();
        }
        MemberFunctionLookupResult result;
        result.functions = found->second;
        result.declared_in = &current;
        return result;
      });
}

MemberFunctionLookupResult lookup_class_scoped_functions(ClassInfo & info,
                                                         const string & name)
{
  return lookup_member_in_hierarchy<MemberFunctionLookupResult>(
      info,
      [&name](ClassInfo & current) -> MemberFunctionLookupResult
      {
        const vector<FunctionBinding *> * found =
            find_direct_function_set(*current.member_scope, name);
        if(!found || found->empty()) {
          return MemberFunctionLookupResult();
        }
        MemberFunctionLookupResult result;
        result.functions = *found;
        result.declared_in = &current;
        return result;
      });
}

MemberFunctionLookupResult lookup_visible_member_functions(ClassInfo & info,
                                                           const string & name)
{
  MemberFunctionLookupResult scoped = lookup_class_scoped_functions(info, name);
  if(!scoped.functions.empty()) {
    return scoped;
  }
  return lookup_member_functions(info, name);
}

MemberFunctionTemplateLookupResult lookup_visible_member_function_templates(ClassInfo & info,
                                                                            const string & name)
{
  return lookup_member_in_hierarchy<MemberFunctionTemplateLookupResult>(
      info,
      [&name](ClassInfo & current) -> MemberFunctionTemplateLookupResult
      {
        const vector<FunctionTemplateDecl *> * found =
            find_direct_function_template_set(*current.member_scope, name);
        if(!found || found->empty()) {
          return MemberFunctionTemplateLookupResult();
        }
        MemberFunctionTemplateLookupResult result;
        result.templates = *found;
        result.declared_in = &current;
        return result;
      });
}

MemberCallableLookupResult lookup_visible_member_callables(ClassInfo & info,
                                                           const string & name)
{
  return lookup_member_in_hierarchy<MemberCallableLookupResult>(
      info,
      [&name](ClassInfo & current) -> MemberCallableLookupResult
      {
        MemberCallableLookupResult result;
        if(current.member_scope) {
          const vector<FunctionBinding *> * found_functions =
              find_direct_function_set(*current.member_scope, name);
          if(found_functions && !found_functions->empty()) {
            result.functions = *found_functions;
          }
          const vector<FunctionTemplateDecl *> * found_templates =
              find_direct_function_template_set(*current.member_scope, name);
          if(found_templates && !found_templates->empty()) {
            result.templates = *found_templates;
          }
        }
        if(result.functions.empty()) {
          map<string, vector<FunctionBinding *> >::iterator found_methods =
              current.methods.find(name);
          if(found_methods != current.methods.end() && !found_methods->second.empty()) {
            result.functions = found_methods->second;
          }
        }
        if(member_lookup_present(result)) {
          result.declared_in = &current;
        }
        return result;
      });
}

MemberClassTemplateLookupResult lookup_member_class_template(SemanticContext & ctx,
                                                             ClassInfo & info,
                                                             const string & name)
{
  return lookup_member_in_hierarchy<MemberClassTemplateLookupResult>(
      info,
      [&ctx, &name](ClassInfo & current) -> MemberClassTemplateLookupResult
      {
        if(!current.member_scope) {
          return MemberClassTemplateLookupResult();
        }
        if(!current.reference_members_collected &&
           !current.reference_member_collection_in_progress) {
          ctx.ensure_class_reference_members(current);
        }
        if(!current.member_scope) {
          return MemberClassTemplateLookupResult();
        }
        map<string, ClassTemplateDecl *>::iterator found =
            current.member_scope->class_templates.find(name);
        if(found == current.member_scope->class_templates.end()) {
          return MemberClassTemplateLookupResult();
        }
        MemberClassTemplateLookupResult result;
        result.class_template = found->second;
        result.declared_in = &current;
        return result;
      });
}

MemberAliasTemplateLookupResult lookup_member_alias_template(SemanticContext & ctx,
                                                             ClassInfo & info,
                                                             const string & name)
{
  return lookup_member_in_hierarchy<MemberAliasTemplateLookupResult>(
      info,
      [&ctx, &name](ClassInfo & current) -> MemberAliasTemplateLookupResult
      {
        if(!current.member_scope) {
          return MemberAliasTemplateLookupResult();
        }
        if(!current.reference_members_collected &&
           !current.reference_member_collection_in_progress) {
          ctx.ensure_class_reference_members(current);
        }
        if(!current.member_scope) {
          return MemberAliasTemplateLookupResult();
        }
        map<string, AliasTemplateDecl *>::iterator found =
            current.member_scope->alias_templates.find(name);
        if(found == current.member_scope->alias_templates.end()) {
          return MemberAliasTemplateLookupResult();
        }
        MemberAliasTemplateLookupResult result;
        result.alias_template = found->second;
        result.declared_in = &current;
        return result;
      });
}

MemberVariableTemplateLookupResult lookup_member_variable_template(SemanticContext & ctx,
                                                                   ClassInfo & info,
                                                                   const string & name)
{
  return lookup_member_in_hierarchy<MemberVariableTemplateLookupResult>(
      info,
      [&ctx, &name](ClassInfo & current) -> MemberVariableTemplateLookupResult
      {
        if(!current.member_scope) {
          return MemberVariableTemplateLookupResult();
        }
        if(!current.reference_members_collected &&
           !current.reference_member_collection_in_progress) {
          ctx.ensure_class_reference_members(current);
        }
        if(!current.member_scope) {
          return MemberVariableTemplateLookupResult();
        }
        map<string, VariableTemplateDecl *>::iterator found =
            current.member_scope->variable_templates.find(name);
        if(found == current.member_scope->variable_templates.end()) {
          return MemberVariableTemplateLookupResult();
        }
        MemberVariableTemplateLookupResult result;
        result.variable_template = found->second;
        result.declared_in = &current;
        return result;
      });
}

bool resolve_qualified_member_target(SemanticContext & ctx,
                                     Scope & scope,
                                     ClassInfo & object_class,
                                     const QualifiedName & member_name,
                                     QualifiedMemberTarget & out,
                                     bool allow_dependent_class_qualifiers)
{
  out = QualifiedMemberTarget();
  out.lookup_name = member_name.name;
  out.target_class = &object_class;

  if(!member_name.rooted && member_name.qualifiers.empty()) {
    out.lookup_name =
        normalize_destructor_member_lookup_name(ctx, scope, *out.target_class, out.lookup_name);
    return true;
  }

  Scope * target =
      resolve_qualified_scope_for_class_or_namespace(ctx,
                                                     scope,
                                                     member_name,
                                                     allow_dependent_class_qualifiers);
  if(!target || !target->class_info) {
    return false;
  }

  out.qualified = true;
  out.target_class = target->class_info;
  out.lookup_name = member_name.name;

  if(out.target_class != &object_class) {
    std::size_t offset = 0;
    MemberAccess access = MA_PUBLIC;
    if(!find_unique_base_path(object_class, out.target_class, offset, access)) {
      return false;
    }
    out.path_offset = offset;
    out.path_access = access;
  }

  out.lookup_name =
      normalize_destructor_member_lookup_name(ctx, scope, *out.target_class, out.lookup_name);

  return true;
}

MemberTypeLookupResult lookup_member_type(SemanticContext & ctx,
                                          ClassInfo & info,
                                          const string & name,
                                          bool ensure_current_reference_members,
                                          Scope * lexical_scope)
{
  const auto inherited_name_is_class_template_type_parameter =
      [](const ClassInfo & owner, const string & member_name) -> bool
  {
    if(!owner.source_template) {
      return false;
    }
    const vector<TemplateParameterInfo> & parameters = owner.source_template->parameters;
    for(size_t i = 0; i < parameters.size(); ++i) {
      const TemplateParameterInfo & parameter = parameters[i];
      if(parameter.kind != TemplateParameterInfo::TP_TYPE) {
        continue;
      }
      if(parameter.name == member_name) {
        return true;
      }
      if(find(parameter.alternate_names.begin(),
              parameter.alternate_names.end(),
              member_name) != parameter.alternate_names.end()) {
        return true;
      }
    }
    return false;
  };

  const auto refine_instantiated_member_type =
      [&ctx, &name](ClassInfo & owner, const TypePtr & type) -> TypePtr
  {
    if(owner.source_template &&
       owner.dependent_instantiation &&
       owner.type &&
       type &&
       ctx.type_depends_on_template_parameter(type)) {
      vector<string> members;
      members.push_back(name);
      return make_dependent_qualified_member_type(
          class_output_qualified_name(owner) + "::" + name,
          owner.type,
          members,
          false);
    }
    if(!owner.source_template ||
       owner.dependent_instantiation ||
       !owner.member_scope ||
       !type ||
       !ctx.type_depends_on_template_parameter(type)) {
      return type;
    }
    TypePtr resolved;
    if(semantic_dependent_type::resolve_instantiated_dependent_type(ctx, *owner.member_scope, type, resolved) &&
       resolved &&
       !ctx.type_depends_on_template_parameter(resolved)) {
      return resolved;
    }
    return type;
  };

  if(info.reference_member_collection_in_progress) {
    MemberTypeLookupResult result;
    if(info.member_scope) {
      map<string, cpp_decl::TypePtr>::const_iterator direct =
          info.member_scope->named_types.find(name);
      if(direct != info.member_scope->named_types.end()) {
        const MemberAccess direct_access =
            named_type_access_for_lookup(*info.member_scope, name);
        if(lexical_scope &&
           !member_access_allowed(lexical_scope,
                                  current_class_scope(*lexical_scope),
                                  current_function_scope(*lexical_scope),
                                  &info,
                                  direct_access,
                                  MA_PUBLIC)) {
          return result;
        }
        if(info.type &&
           (info.source_template || info.dependent_instantiation) &&
           (!direct->second || ctx.type_depends_on_template_parameter(direct->second))) {
          vector<string> members;
          members.push_back(name);
          result.type = make_dependent_qualified_member_type(
              class_output_qualified_name(info) + "::" + name,
              info.type,
              members,
              false);
        } else {
          result.type = direct->second;
        }
        result.declared_in = &info;
        return result;
      }
    }
  }

  if(ensure_current_reference_members &&
     !info.reference_member_collection_in_progress &&
     !info.reference_members_collected) {
    ctx.ensure_class_reference_members(info);
  }

  map<string, cpp_decl::TypePtr>::const_iterator direct =
      info.member_scope->named_types.find(name);
  if(direct != info.member_scope->named_types.end()) {
    TypePtr direct_type = direct->second;
    TypePtr resolved_deferred_alias;
    if(semantic_class_model::resolve_deferred_class_alias(ctx,
                                                          info,
                                                          name,
                                                          resolved_deferred_alias) &&
       resolved_deferred_alias) {
      direct_type = resolved_deferred_alias;
    }
    const MemberAccess direct_access =
        named_type_access_for_lookup(*info.member_scope, name);
    if(lexical_scope &&
       !member_access_allowed(lexical_scope,
                              current_class_scope(*lexical_scope),
                              current_function_scope(*lexical_scope),
                              &info,
                              direct_access,
                              MA_PUBLIC)) {
      return MemberTypeLookupResult();
    }
    MemberTypeLookupResult result;
    result.type = refine_instantiated_member_type(info, direct_type);
    result.declared_in = &info;
    return result;
  }

  set<ClassInfo *> visited_virtual;
  vector<ClassInfo *> candidates;
  vector<MemberAccess> candidate_path_access;
  vector<pair<ClassInfo *, MemberAccess> > stack;
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].type) {
      stack.push_back(make_pair(info.bases[i].type, info.bases[i].access));
    }
  }

  while(!stack.empty()) {
    const pair<ClassInfo *, MemberAccess> current_entry = stack.back();
    stack.pop_back();
    ClassInfo * current = current_entry.first;
    const MemberAccess current_path_access = current_entry.second;
    if(!current) {
      continue;
    }

    if(current->reference_member_collection_in_progress) {
      continue;
    }

    if(!current->reference_members_collected) {
      ctx.ensure_class_reference_members(*current);
    }

    map<string, cpp_decl::TypePtr>::const_iterator found =
        current->member_scope->named_types.find(name);
    if(found != current->member_scope->named_types.end() &&
       !inherited_name_is_class_template_type_parameter(*current, name)) {
      candidates.push_back(current);
      candidate_path_access.push_back(current_path_access);
      continue;
    }

    for(size_t i = 0; i < current->bases.size(); ++i) {
      BaseInfo & base = current->bases[i];
      if(base.is_virtual && !visited_virtual.insert(base.type).second) {
        continue;
      }
      if(base.type) {
        stack.push_back(make_pair(base.type,
                                  combine_member_access(current_path_access, base.access)));
      }
    }
  }

  if(candidates.empty()) {
    return MemberTypeLookupResult();
  }

  ClassInfo * declared_in = candidates[0];
  TypePtr declared_type = declared_in->member_scope->named_types.find(name)->second;
  for(size_t i = 1; i < candidates.size(); ++i) {
    if(candidates[i] != declared_in) {
      TypePtr candidate_type = candidates[i]->member_scope->named_types.find(name)->second;
      ClassInfo * declared_info = ctx.class_info_for_type(declared_type);
      ClassInfo * candidate_info = ctx.class_info_for_type(candidate_type);
      const bool same_class_type =
          declared_info && candidate_info && declared_info == candidate_info;
      const bool same_type_text =
          declared_type && candidate_type &&
          describe_type(declared_type) == describe_type(candidate_type);
      if(!same_class_type && !same_type_text) {
        ostringstream out;
        out << "ambiguous member lookup";
        out << " [root " << info.qualified_name << "]";
        out << " [member " << name << "]";
        out << " [candidates " << describe_class_candidate_list(candidates) << "]";
        throw logic_error(out.str());
      }
    }
  }

  MemberTypeLookupResult result;
  result.path_access = candidate_path_access[0];
  const MemberAccess member_access =
      named_type_access_for_lookup(*declared_in->member_scope, name);
  if(lexical_scope &&
     !member_access_allowed(lexical_scope,
                            current_class_scope(*lexical_scope),
                            current_function_scope(*lexical_scope),
                            declared_in,
                            member_access,
                            result.path_access)) {
    return MemberTypeLookupResult();
  }
  result.type = refine_instantiated_member_type(
      *declared_in,
      declared_in->member_scope->named_types.find(name)->second);
  result.declared_in = declared_in;
  return result;
}

bool function_has_friend_access(const FunctionBinding * current_function,
                                const ClassInfo * declared_in)
{
  if(!current_function || !declared_in) {
    return false;
  }
  const auto same_class_friend_entity =
      [](const ClassInfo * lhs, const ClassInfo * rhs) -> bool
  {
    if(lhs == rhs) {
      return true;
    }
    if(!lhs || !rhs) {
      return false;
    }
    if(lhs->source_template && rhs->source_template) {
      return lhs->source_template == rhs->source_template;
    }
    const auto matches_primary_to_instantiation =
        [](const ClassInfo * primary, const ClassInfo * instantiated) -> bool
    {
      if(!primary || !instantiated || primary->source_template || !instantiated->source_template ||
         !primary->enclosing_scope || !instantiated->source_template->declaring_scope) {
        return false;
      }
      return primary->name == instantiated->source_template->name &&
             inline_namespace_collapsed_scope_name(primary->enclosing_scope) ==
                 inline_namespace_collapsed_scope_name(
                     instantiated->source_template->declaring_scope);
    };
    return matches_primary_to_instantiation(lhs, rhs) ||
           matches_primary_to_instantiation(rhs, lhs);
  };
  if(current_function->source_template) {
    for(size_t i = 0; i < current_function->source_template->friend_access_classes.size(); ++i) {
      const ClassInfo * friend_class =
          current_function->source_template->friend_access_classes[i];
      if(same_class_friend_entity(friend_class, declared_in)) {
        return true;
      }
    }
    for(size_t i = 0; i < declared_in->friend_access_function_templates.size(); ++i) {
      FunctionTemplateDecl * access_template =
          declared_in->friend_access_function_templates[i];
      if(same_inline_namespace_function_template_entity(
             access_template,
             current_function->source_template)) {
        return true;
      }
    }
    for(size_t i = 0; i < declared_in->friend_function_templates.size(); ++i) {
      if(same_inline_namespace_function_template_entity(
             declared_in->friend_function_templates[i],
             current_function->source_template)) {
        return true;
      }
    }
  }
  for(size_t i = 0; i < declared_in->friend_functions.size(); ++i) {
    FunctionBinding * friend_function = declared_in->friend_functions[i];
    const bool same_binding = friend_function == current_function;
    const bool same_entity =
        friend_function &&
        same_inline_namespace_function_entity(*friend_function, *current_function);
    const bool same_source_template =
        friend_function &&
        friend_function->source_template &&
        current_function->source_template &&
        friend_function->source_template == current_function->source_template;
    const bool same_template_friend_reference =
        friend_function &&
        !friend_function->source_template &&
        current_function->source_template &&
        !friend_function->owner_class &&
        !current_function->owner_class &&
        friend_function->name.find('<') != string::npos &&
        template_base_name(canonical_function_lookup_name(friend_function->name)) ==
            template_base_name(
                canonical_function_lookup_name(current_function->source_template->name));
    if(same_binding || same_entity || same_source_template ||
       same_template_friend_reference) {
      return true;
    }
  }
  return false;
}

bool class_matches_friend_name(const ClassInfo * current_class,
                               const std::string & friend_name)
{
  if(!current_class || friend_name.empty()) {
    return false;
  }

  const string normalized_friend = strip_friend_type_prefixes(friend_name);
  const size_t qualifier_pos = normalized_friend.rfind("::");
  if(qualifier_pos != string::npos) {
    const string friend_owner = normalized_friend.substr(0, qualifier_pos);
    const string friend_member = normalized_friend.substr(qualifier_pos + 2);
    if(friend_owner_matches_current_class(current_class, friend_owner) &&
       (friend_member == "self" ||
        friend_member_alias_resolves_to_current_class(current_class, friend_member))) {
      return true;
    }
  }

  const string unqualified_friend = template_base_name(normalized_friend);
  if(current_class->source_template &&
     current_class->source_template->name == unqualified_friend) {
    return true;
  }
  if(template_base_name(current_class->name) == unqualified_friend) {
    return true;
  }
  return template_base_name(current_class->qualified_name) == unqualified_friend;
}

bool class_has_friend_class_access(const ClassInfo * current_class,
                                   const ClassInfo * declared_in)
{
  if(!current_class || !declared_in) {
    return false;
  }

  for(size_t i = 0; i < declared_in->friend_class_names.size(); ++i) {
    if(class_matches_friend_name(current_class, declared_in->friend_class_names[i])) {
      return true;
    }
  }
  return false;
}

bool scope_has_friend_class_access(const Scope * scope, const ClassInfo * declared_in)
{
  for(const Scope * current = scope; current; current = current->parent) {
    if(class_has_friend_class_access(current->class_info, declared_in)) {
      return true;
    }
  }
  return false;
}

bool scope_has_class_access(const Scope * scope,
                            const ClassInfo * declared_in,
                            MemberAccess member_access)
{
  for(const Scope * current = scope; current; current = current->parent) {
    const ClassInfo * current_class = current->class_info;
    if(!current_class) {
      continue;
    }
    if(current_class == declared_in) {
      return true;
    }
    if(is_same_or_derived(current_class, declared_in)) {
      return member_access != MA_PRIVATE;
    }
  }
  return false;
}

bool member_access_allowed(const Scope * lexical_scope,
                           const ClassInfo * current_class,
                           const FunctionBinding * current_function,
                           const ClassInfo * declared_in,
                           MemberAccess member_access,
                           MemberAccess path_access)
{
  auto class_grants_access = [&](const ClassInfo * access_class) -> bool
  {
    if(!access_class) {
      return false;
    }
    if(access_class == declared_in) {
      return true;
    }
    if(is_same_or_derived(access_class, declared_in)) {
      return member_access != MA_PRIVATE;
    }
    if(class_has_friend_class_access(access_class, declared_in)) {
      return true;
    }
    return false;
  };

  if(class_grants_access(current_class)) {
    return true;
  }

  if(current_function && class_grants_access(current_function->owner_class)) {
    return true;
  }

  if(current_function && class_grants_access(current_function->lexical_access_class)) {
    return true;
  }

  const bool allow_scope_walk =
      !current_function ||
      (!current_function->synthesized &&
       !current_function->is_defaulted &&
       !current_function->is_aggregate_constructor);
  const Scope * access_scope =
      allow_scope_walk ?
          (lexical_scope ? lexical_scope :
           (current_function ? current_function->declaration_scope : nullptr)) :
          nullptr;
  if(current_function && access_scope &&
     (scope_has_friend_class_access(access_scope, declared_in) ||
      scope_has_class_access(access_scope,
                            declared_in,
                            member_access))) {
    return true;
  }

  if(function_has_friend_access(current_function, declared_in)) {
    return true;
  }

  if(current_function &&
     current_function->lexical_access_function &&
     function_has_friend_access(current_function->lexical_access_function, declared_in)) {
    return true;
  }

  return member_access == MA_PUBLIC && path_access == MA_PUBLIC;
}

bool member_access_allowed_through_object(const Scope * lexical_scope,
                                          const ClassInfo * current_class,
                                          const FunctionBinding * current_function,
                                          const ClassInfo * object_class,
                                          const ClassInfo * declared_in,
                                          MemberAccess member_access,
                                          MemberAccess path_access)
{
  if(member_access_allowed(lexical_scope,
                           current_class,
                           current_function,
                           declared_in,
                           member_access,
                           path_access)) {
    return true;
  }

  if(!object_class || !declared_in ||
     object_class == declared_in ||
     member_access == MA_PRIVATE ||
     !is_same_or_derived(object_class, declared_in)) {
    return false;
  }

  if(class_has_friend_class_access(current_class, object_class)) {
    return true;
  }
  if(current_function &&
     class_has_friend_class_access(current_function->owner_class, object_class)) {
    return true;
  }
  if(current_function &&
     class_has_friend_class_access(current_function->lexical_access_class, object_class)) {
    return true;
  }
  if(current_function && function_has_friend_access(current_function, object_class)) {
    return true;
  }
  if(current_function &&
     current_function->lexical_access_function &&
     function_has_friend_access(current_function->lexical_access_function, object_class)) {
    return true;
  }
  return scope_has_friend_class_access(lexical_scope, object_class);
}

const vector<FunctionBinding *> * find_direct_function_set_by_canonical_name(
    const Scope & scope,
    const string & canonical_name)
{
  map<string, vector<FunctionBinding *> >::const_iterator found =
      scope.function_sets.find(canonical_name);
  return found == scope.function_sets.end() ? nullptr : &found->second;
}

size_t direct_function_lookup_dependency_token(const Scope & scope)
{
  size_t seed = scope.direct_function_lookup_cache_epoch;
  hash_combine(seed, scope.namespace_children.size());
  if(scope.class_info) {
    return seed;
  }
  for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
    const Scope & child = *scope.namespace_children[i];
    if(!namespace_child_injected_for_direct_lookup(child)) {
      continue;
    }
    hash_combine(seed, reinterpret_cast<size_t>(&child));
    hash_combine(seed, child.inline_namespace ? 1 : 0);
    hash_combine(seed, std::hash<string>()(child.name));
    hash_combine(seed, direct_function_lookup_dependency_token(child));
  }
  return seed;
}

vector<FunctionBinding *> lookup_direct_functions_by_canonical_name(Scope & scope,
                                                                    const string & canonical_name)
{
  const size_t dependency_token = direct_function_lookup_dependency_token(scope);
  map<string, Scope::DirectFunctionLookupCacheEntry>::const_iterator cached =
      scope.cached_direct_function_lookups.find(canonical_name);
  if(cached != scope.cached_direct_function_lookups.end() &&
     cached->second.dependency_token == dependency_token) {
    return cached->second.functions;
  }

  const vector<FunctionBinding *> * found =
      find_direct_function_set_by_canonical_name(scope, canonical_name);
  vector<FunctionBinding *> out;
  if(found) {
    append_unique_functions(out, *found);
  }

  if(!scope.class_info) {
    for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
      Scope & child = *scope.namespace_children[i];
      if(!namespace_child_injected_for_direct_lookup(child)) {
        continue;
      }
      append_unique_functions(out, lookup_direct_functions_by_canonical_name(child,
                                                                              canonical_name));
    }
  }
  Scope::DirectFunctionLookupCacheEntry & entry =
      scope.cached_direct_function_lookups[canonical_name];
  entry.dependency_token = dependency_token;
  entry.functions = out;
  return out;
}

vector<FunctionBinding *> lookup_direct_functions(Scope & scope, const string & name)
{
  const string canonical_name = canonical_function_lookup_name(name);
  return lookup_direct_functions_by_canonical_name(scope, canonical_name);
}

ClassTemplateDecl * lookup_direct_class_template(Scope & scope, const string & name)
{
  map<string, ClassTemplateDecl *>::iterator found = scope.class_templates.find(name);
  return found == scope.class_templates.end() ? nullptr : found->second;
}

AliasTemplateDecl * lookup_direct_alias_template(Scope & scope, const string & name)
{
  map<string, AliasTemplateDecl *>::iterator found = scope.alias_templates.find(name);
  return found == scope.alias_templates.end() ? nullptr : found->second;
}

VariableTemplateDecl * lookup_direct_variable_template(Scope & scope, const string & name)
{
  map<string, VariableTemplateDecl *>::iterator found = scope.variable_templates.find(name);
  return found == scope.variable_templates.end() ? nullptr : found->second;
}

void collect_direct_function_templates(Scope & scope,
                                       const string & name,
                                       vector<FunctionTemplateDecl *> & out)
{
  const vector<FunctionTemplateDecl *> * found = find_direct_function_template_set(scope, name);
  if(found) {
    if(!scope.class_info) {
      append_unique_function_templates(out, *found);
    } else {
      const bool has_exact_owner =
          any_of(found->begin(),
                 found->end(),
                 [&scope](FunctionTemplateDecl * decl)
                 {
                   return decl && decl->declaring_scope &&
                          decl->declaring_scope->class_info == scope.class_info;
                 });
      if(!has_exact_owner) {
        append_unique_function_templates(out, *found);
      } else {
        for(size_t i = 0; i < found->size(); ++i) {
          FunctionTemplateDecl * decl = (*found)[i];
          if(decl && decl->declaring_scope &&
             decl->declaring_scope->class_info &&
             decl->declaring_scope->class_info != scope.class_info) {
            continue;
          }
          out.push_back(decl);
        }
      }
    }
  }

  if(!scope.class_info) {
    for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
      Scope & child = *scope.namespace_children[i];
      if(!namespace_child_injected_for_direct_lookup(child)) {
        continue;
      }
      collect_direct_function_templates(child, name, out);
    }
  }
}

vector<FunctionTemplateDecl *> lookup_direct_function_templates(Scope & scope,
                                                                const string & name)
{
  vector<FunctionTemplateDecl *> out;
  collect_direct_function_templates(scope, name, out);
  return out;
}

void append_unique_functions(vector<FunctionBinding *> & out,
                             const vector<FunctionBinding *> & in)
{
  if(out.size() + in.size() >= 8) {
    vector<FunctionLookupDedupeKey> out_keys;
    out_keys.reserve(out.size() + in.size());
    for(size_t i = 0; i < out.size(); ++i) {
      out_keys.push_back(function_lookup_dedupe_key(out[i]));
    }

    for(size_t i = 0; i < in.size(); ++i) {
      const FunctionLookupDedupeKey candidate_key = function_lookup_dedupe_key(in[i]);
      bool duplicate = false;
      for(size_t j = 0; j < out.size(); ++j) {
        if(!function_lookup_dedupe_key_maybe_equal(out_keys[j], candidate_key)) {
          continue;
        }
        if(out[j] == in[i] ||
           (out[j] && in[i] && same_inline_namespace_function_entity(*out[j], *in[i]))) {
          duplicate = true;
          break;
        }
      }
      if(!duplicate) {
        out.push_back(in[i]);
        out_keys.push_back(candidate_key);
      }
    }
    return;
  }

  for(size_t i = 0; i < in.size(); ++i) {
    bool duplicate = false;
    for(size_t j = 0; j < out.size(); ++j) {
      if(out[j] == in[i] ||
         (out[j] && in[i] && same_inline_namespace_function_entity(*out[j], *in[i]))) {
        duplicate = true;
        break;
      }
    }
    if(!duplicate) {
      out.push_back(in[i]);
    }
  }
}

void append_unique_function_templates(vector<FunctionTemplateDecl *> & out,
                                      const vector<FunctionTemplateDecl *> & in)
{
  if(out.size() + in.size() >= 8) {
    vector<FunctionTemplateLookupDedupeKey> out_keys;
    out_keys.reserve(out.size() + in.size());
    for(size_t i = 0; i < out.size(); ++i) {
      out_keys.push_back(function_template_lookup_dedupe_key(out[i]));
    }

    for(size_t i = 0; i < in.size(); ++i) {
      const FunctionTemplateLookupDedupeKey candidate_key =
          function_template_lookup_dedupe_key(in[i]);
      bool duplicate = false;
      for(size_t j = 0; j < out.size(); ++j) {
        if(!function_template_lookup_dedupe_key_maybe_equal(out_keys[j],
                                                            candidate_key)) {
          continue;
        }
        if(out[j] == in[i] ||
           same_inline_namespace_function_template_entity(out[j], in[i])) {
          duplicate = true;
          break;
        }
      }
      if(!duplicate) {
        out.push_back(in[i]);
        out_keys.push_back(candidate_key);
      }
    }
    return;
  }

  for(size_t i = 0; i < in.size(); ++i) {
    bool duplicate = false;
    for(size_t j = 0; j < out.size(); ++j) {
      if(out[j] == in[i] ||
         same_inline_namespace_function_template_entity(out[j], in[i])) {
        duplicate = true;
        break;
      }
    }
    if(!duplicate) {
      out.push_back(in[i]);
    }
  }
}

void append_unique_scopes(vector<Scope *> & out, Scope * scope)
{
  if(scope && find(out.begin(), out.end(), scope) == out.end()) {
    out.push_back(scope);
  }
}

void lookup_functions_from_using_directives(Scope & scope,
                                            const string & name,
                                            set<const Scope *> & visited,
                                            vector<FunctionBinding *> & out)
{
  if(scope.using_directives.empty()) {
    return;
  }
  if(!visited.insert(&scope).second) {
    return;
  }

  for(size_t i = 0; i < scope.using_directives.size(); ++i) {
    Scope * imported = scope.using_directives[i];
    append_unique_functions(out, lookup_direct_functions(*imported, name));
    lookup_functions_from_using_directives(*imported, name, visited, out);
  }
}

void lookup_function_templates_from_using_directives(
    Scope & scope,
    const string & name,
    set<const Scope *> & visited,
    vector<FunctionTemplateDecl *> & out)
{
  if(scope.using_directives.empty()) {
    return;
  }
  if(!visited.insert(&scope).second) {
    return;
  }

  for(size_t i = 0; i < scope.using_directives.size(); ++i) {
    Scope * imported = scope.using_directives[i];
    append_unique_function_templates(out, lookup_direct_function_templates(*imported, name));
    lookup_function_templates_from_using_directives(*imported, name, visited, out);
  }
}

ValueLookupFromUsingDirectivesResult lookup_value_from_using_directives(
    Scope & scope,
    const string & name,
    set<const Scope *> & visited)
{
  if(scope.using_directives.empty()) {
    return ValueLookupFromUsingDirectivesResult();
  }
  if(!visited.insert(&scope).second) {
    return ValueLookupFromUsingDirectivesResult();
  }

  const ValueBinding * found = nullptr;
  for(size_t i = 0; i < scope.using_directives.size(); ++i) {
    Scope * imported = scope.using_directives[i];
    const ValueBinding * direct = lookup_direct_value(*imported, name);
    if(direct) {
      if(!found) {
        found = direct;
      } else if(!same_value_binding_entity(found, direct)) {
        return ValueLookupFromUsingDirectivesResult::make_ambiguous();
      }
    }
    ValueLookupFromUsingDirectivesResult nested =
        lookup_value_from_using_directives(*imported, name, visited);
    if(nested.ambiguous) {
      return nested;
    }
    if(nested.binding) {
      if(!found) {
        found = nested.binding;
      } else if(!same_value_binding_entity(found, nested.binding)) {
        return ValueLookupFromUsingDirectivesResult::make_ambiguous();
      }
    }
  }

  return ValueLookupFromUsingDirectivesResult(found);
}

void lookup_functions_in_scopes(const vector<Scope *> & scopes,
                                const string & name,
                                vector<FunctionBinding *> & out)
{
  set<const Scope *> visited;
  for(size_t i = 0; i < scopes.size(); ++i) {
    if(!scopes[i]) {
      continue;
    }
    append_unique_functions(out, lookup_direct_functions(*scopes[i], name));
    lookup_functions_from_using_directives(*scopes[i], name, visited, out);
  }
}

void lookup_function_templates_in_scopes(const vector<Scope *> & scopes,
                                         const string & name,
                                         vector<FunctionTemplateDecl *> & out)
{
  set<const Scope *> visited;
  for(size_t i = 0; i < scopes.size(); ++i) {
    if(!scopes[i]) {
      continue;
    }
    append_unique_function_templates(out, lookup_direct_function_templates(*scopes[i], name));
    lookup_function_templates_from_using_directives(*scopes[i], name, visited, out);
  }
}

namespace {

bool declaration_scope_belongs_to_adl_scope(const Scope * declaration_scope,
                                            const Scope & associated_scope)
{
  return declaration_scope &&
         inline_namespace_collapsed_scope_name(declaration_scope) ==
             inline_namespace_collapsed_scope_name(&associated_scope);
}

void append_adl_direct_functions(Scope & scope,
                                 const string & name,
                                 vector<FunctionBinding *> & out)
{
  const vector<FunctionBinding *> * found = find_direct_function_set(scope, name);
  vector<FunctionBinding *> candidates;
  if(found) {
    for(size_t i = 0; i < found->size(); ++i) {
      FunctionBinding * binding = (*found)[i];
      if(binding &&
         declaration_scope_belongs_to_adl_scope(binding->declaration_scope, scope)) {
        candidates.push_back(binding);
      }
    }
  }
  append_unique_functions(out, candidates);

  for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
    Scope & child = *scope.namespace_children[i];
    if(child.inline_namespace) {
      append_adl_direct_functions(child, name, out);
    }
  }
}

void append_adl_direct_function_templates(Scope & scope,
                                          const string & name,
                                          vector<FunctionTemplateDecl *> & out)
{
  const vector<FunctionTemplateDecl *> * found =
      find_direct_function_template_set(scope, name);
  if(found) {
    append_unique_function_templates(out, *found);
  }

  for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
    Scope & child = *scope.namespace_children[i];
    if(child.inline_namespace) {
      append_adl_direct_function_templates(child, name, out);
    }
  }
}

}  // namespace

void lookup_adl_functions_in_scopes(const vector<Scope *> & scopes,
                                    const string & name,
                                    vector<FunctionBinding *> & out)
{
  for(size_t i = 0; i < scopes.size(); ++i) {
    if(scopes[i]) {
      append_adl_direct_functions(*scopes[i], name, out);
    }
  }
}

void lookup_adl_function_templates_in_scopes(
    const vector<Scope *> & scopes,
    const string & name,
    vector<FunctionTemplateDecl *> & out)
{
  for(size_t i = 0; i < scopes.size(); ++i) {
    if(scopes[i]) {
      append_adl_direct_function_templates(*scopes[i], name, out);
    }
  }
}

Scope * lookup_namespace_name(Scope & scope, const QualifiedName & qualified)
{
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    QualifiedName split_name;
    if(qualified.name.find("::") != string::npos &&
       semantic_utils::split_qualified_name_text(qualified.name, split_name) &&
       (split_name.rooted || !split_name.qualifiers.empty())) {
      return lookup_namespace_name(scope, split_name);
    }
    return lookup_namespace_from_scope(scope, qualified.name);
  }

  Scope * current = qualified.rooted ? root_scope(scope) :
      lookup_namespace_from_scope(scope, qualified.qualifiers[0]);
  size_t next = qualified.rooted ? 0 : 1;
  if(!current) {
    return nullptr;
  }

  while(next < qualified.qualifiers.size()) {
    current = lookup_namespace_member_in_qualified_scope(
        *current,
        qualified.qualifiers[next]);
    if(!current) {
      return nullptr;
    }
    ++next;
  }

  if(qualified.name.empty()) {
    return current;
  }

  return lookup_namespace_member_in_qualified_scope(*current, qualified.name);
}

namespace {

bool collect_associated_namespace_scopes_for_type_impl(
    SemanticContext & ctx,
    const TypePtr & type,
    set<const Type *> & visited_types,
    set<const ClassInfo *> & visited_classes,
    vector<Scope *> & out);

struct AdlTypePtrAddressHash
{
  size_t operator()(const TypePtr & type) const
  {
    return std::hash<const Type *>()(type.get());
  }
};

struct AdlTypePtrAddressEqual
{
  bool operator()(const TypePtr & lhs, const TypePtr & rhs) const
  {
    return lhs.get() == rhs.get();
  }
};

typedef unordered_map<TypePtr,
                      vector<Scope *>,
                      AdlTypePtrAddressHash,
                      AdlTypePtrAddressEqual>
    AssociatedNamespaceScopeCache;

TypePtr associated_namespace_scope_cache_key_type(const TypePtr & type)
{
  TypePtr base = remove_reference_type(type);
  if(!base) {
    base = type;
  }
  return strip_top_level_cv(base);
}

AssociatedNamespaceScopeCache & associated_namespace_scope_cache()
{
  static AssociatedNamespaceScopeCache cache;
  return cache;
}

void append_associated_namespace_scope_for_declaration_scope(Scope * scope,
                                                             vector<Scope *> & out)
{
  Scope * namespace_scope = nullptr;
  for(Scope * current = scope; current; current = current->parent) {
    if(current->namespace_scope) {
      namespace_scope = current;
      break;
    }
  }

  while(namespace_scope &&
        namespace_scope->namespace_scope &&
        namespace_scope->inline_namespace) {
    append_unique_scopes(out, namespace_scope);
    namespace_scope = namespace_scope->parent;
  }
  if(namespace_scope && namespace_scope->namespace_scope) {
    append_unique_scopes(out, namespace_scope);
  }
}

bool collect_associated_namespace_scopes_for_template_argument(
    SemanticContext & ctx,
    const TemplateArgument & argument,
    set<const Type *> & visited_types,
    set<const ClassInfo *> & visited_classes,
    vector<Scope *> & out)
{
  bool cacheable = true;
  if(argument.type) {
    cacheable =
        collect_associated_namespace_scopes_for_type_impl(
            ctx, argument.type, visited_types, visited_classes, out) &&
        cacheable;
  }

  Scope * declaring_scope = nullptr;
  if(ClassTemplateDecl * class_template =
         template_model::template_argument_class_template(argument)) {
    declaring_scope = class_template->declaring_scope;
  } else if(AliasTemplateDecl * alias_template =
                template_model::template_argument_alias_template(argument)) {
    declaring_scope = alias_template->declaring_scope;
  }

  append_associated_namespace_scope_for_declaration_scope(declaring_scope, out);
  return cacheable;
}

bool collect_associated_namespace_scopes_for_type_impl(
    SemanticContext & ctx,
    const TypePtr & type,
    set<const Type *> & visited_types,
    set<const ClassInfo *> & visited_classes,
    vector<Scope *> & out)
{
  TypePtr base = remove_reference_type(type);
  if(!base) {
    base = type;
  }
  base = strip_top_level_cv(base);
  if(!base || !visited_types.insert(base.get()).second) {
    return true;
  }

  bool cacheable = true;
  switch(base->kind) {
    case Type::TK_CV:
    case Type::TK_ATOMIC:
    case Type::TK_POINTER:
    case Type::TK_BLOCK_POINTER:
    case Type::TK_LVALUE_REFERENCE:
    case Type::TK_RVALUE_REFERENCE:
    case Type::TK_ARRAY:
      cacheable =
          collect_associated_namespace_scopes_for_type_impl(
              ctx, base->inner, visited_types, visited_classes, out) &&
          cacheable;
      break;
    case Type::TK_MEMBER_POINTER:
      cacheable =
          collect_associated_namespace_scopes_for_type_impl(
              ctx, base->owner, visited_types, visited_classes, out) &&
          cacheable;
      cacheable =
          collect_associated_namespace_scopes_for_type_impl(
              ctx, base->inner, visited_types, visited_classes, out) &&
          cacheable;
      break;
    case Type::TK_FUNCTION:
      cacheable =
          collect_associated_namespace_scopes_for_type_impl(
              ctx, base->inner, visited_types, visited_classes, out) &&
          cacheable;
      for(size_t i = 0; i < base->params.size(); ++i) {
        cacheable =
            collect_associated_namespace_scopes_for_type_impl(
                ctx, base->params[i], visited_types, visited_classes, out) &&
            cacheable;
      }
      break;
    case Type::TK_FUNDAMENTAL:
      break;
    case Type::TK_NAMED:
      if(is_named_enum_type(ctx, base)) {
        append_associated_namespace_scope_for_declaration_scope(
            ctx.scope_for_type(base), out);
      }
      break;
  }

  ClassInfo * info = ctx.class_info_for_type(base);
  if(!info || !visited_classes.insert(info).second) {
    return cacheable;
  }

  if(info->template_instantiation_in_progress ||
     info->full_member_collection_in_progress ||
     info->reference_member_collection_in_progress ||
     (!info->complete && !info->reference_members_collected)) {
    cacheable = false;
  }

  append_associated_namespace_scope_for_declaration_scope(info->enclosing_scope, out);

  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(info->bases[i].type) {
      cacheable =
          collect_associated_namespace_scopes_for_type_impl(
              ctx, info->bases[i].type->type, visited_types, visited_classes, out) &&
          cacheable;
    }
  }

  for(size_t i = 0; i < info->instantiation_arguments.size(); ++i) {
    cacheable =
        collect_associated_namespace_scopes_for_template_argument(
            ctx, info->instantiation_arguments[i], visited_types, visited_classes, out) &&
        cacheable;
  }
  return cacheable;
}

}  // namespace

void collect_associated_namespace_scopes_for_type(SemanticContext & ctx,
                                                  const TypePtr & type,
                                                  vector<Scope *> & out)
{
  TypePtr key_type = associated_namespace_scope_cache_key_type(type);
  semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters();
  if(key_type) {
    AssociatedNamespaceScopeCache & cache = associated_namespace_scope_cache();
    AssociatedNamespaceScopeCache::const_iterator cached = cache.find(key_type);
    if(cached != cache.end()) {
      if(counters) {
        ++counters->adl_associated_scope_cache_hits;
      }
      for(size_t i = 0; i < cached->second.size(); ++i) {
        append_unique_scopes(out, cached->second[i]);
      }
      return;
    }
    if(counters) {
      ++counters->adl_associated_scope_cache_misses;
    }
    vector<Scope *> scopes;
    set<const Type *> visited_types;
    set<const ClassInfo *> visited_classes;
    const bool cacheable =
        collect_associated_namespace_scopes_for_type_impl(
            ctx, key_type, visited_types, visited_classes, scopes);
    for(size_t i = 0; i < scopes.size(); ++i) {
      append_unique_scopes(out, scopes[i]);
    }
    if(cacheable) {
      if(cache.size() > 262144) {
        if(counters) {
          ++counters->adl_associated_scope_cache_clears;
        }
        cache.clear();
      }
      cache[key_type] = scopes;
      if(counters) {
        ++counters->adl_associated_scope_cache_entries;
      }
      return;
    }
    if(counters) {
      ++counters->adl_associated_scope_cache_uncacheable;
    }
    return;
  }

  set<const Type *> visited_types;
  set<const ClassInfo *> visited_classes;
  collect_associated_namespace_scopes_for_type_impl(
      ctx, type, visited_types, visited_classes, out);
}

namespace {

void collect_associated_friend_functions_for_class(const ClassInfo * info,
                                                   const string & name,
                                                   set<const ClassInfo *> & visited,
                                                   vector<FunctionBinding *> & out)
{
  if(!info || !visited.insert(info).second) {
    return;
  }

  const string lookup_name =
      unqualified_name_text(canonical_function_lookup_name(name));
  for(size_t i = 0; i < info->friend_functions.size(); ++i) {
    FunctionBinding * binding = info->friend_functions[i];
    if(!binding ||
       unqualified_name_text(canonical_function_lookup_name(binding->name)) != lookup_name) {
      continue;
    }
    append_unique_functions(out, vector<FunctionBinding *>(1, binding));
  }

  for(size_t i = 0; i < info->bases.size(); ++i) {
    collect_associated_friend_functions_for_class(info->bases[i].type,
                                                  name,
                                                  visited,
                                                  out);
  }
}

void collect_associated_friend_function_templates_for_class(
    const ClassInfo * info,
    const string & name,
    set<const ClassInfo *> & visited,
    vector<FunctionTemplateDecl *> & out)
{
  if(!info || !visited.insert(info).second) {
    return;
  }

  const string lookup_name =
      unqualified_name_text(canonical_function_lookup_name(name));
  for(size_t i = 0; i < info->friend_function_templates.size(); ++i) {
    FunctionTemplateDecl * decl = info->friend_function_templates[i];
    if(!decl ||
       unqualified_name_text(canonical_function_lookup_name(decl->name)) != lookup_name) {
      continue;
    }
    append_unique_function_templates(out, vector<FunctionTemplateDecl *>(1, decl));
  }

  for(size_t i = 0; i < info->bases.size(); ++i) {
    collect_associated_friend_function_templates_for_class(info->bases[i].type,
                                                           name,
                                                           visited,
                                                           out);
  }
}

struct AssociatedFriendLookupCacheKey
{
  TypePtr type;
  string name;

  bool operator==(const AssociatedFriendLookupCacheKey & other) const
  {
    return type.get() == other.type.get() && name == other.name;
  }
};

struct AssociatedFriendLookupCacheKeyHash
{
  size_t operator()(const AssociatedFriendLookupCacheKey & key) const
  {
    size_t seed = std::hash<const Type *>()(key.type.get());
    seed ^= std::hash<string>()(key.name) + 0x9e3779b97f4a7c15ULL +
            (seed << 6) + (seed >> 2);
    return seed;
  }
};

struct AssociatedFriendLookupCacheEntry
{
  vector<FunctionBinding *> functions;
  vector<FunctionTemplateDecl *> templates;
};

typedef unordered_map<AssociatedFriendLookupCacheKey,
                      AssociatedFriendLookupCacheEntry,
                      AssociatedFriendLookupCacheKeyHash>
    AssociatedFriendLookupCache;

AssociatedFriendLookupCache & associated_friend_lookup_cache()
{
  static AssociatedFriendLookupCache cache;
  return cache;
}

bool associated_friend_lookup_cacheable_class(const ClassInfo * info,
                                              set<const ClassInfo *> & visited)
{
  if(!info || !visited.insert(info).second) {
    return true;
  }
  if(info->template_instantiation_in_progress ||
     info->full_member_collection_in_progress ||
     info->reference_member_collection_in_progress ||
     (!info->complete && !info->reference_members_collected)) {
    return false;
  }
  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(!associated_friend_lookup_cacheable_class(info->bases[i].type, visited)) {
      return false;
    }
  }
  return true;
}

bool make_associated_friend_lookup_cache_key(
    const TypePtr & type,
    const string & name,
    AssociatedFriendLookupCacheKey & out)
{
  TypePtr key_type = associated_namespace_scope_cache_key_type(type);
  if(!key_type) {
    return false;
  }
  out.type = key_type;
  out.name = unqualified_name_text(canonical_function_lookup_name(name));
  return !out.name.empty();
}

void lookup_associated_friend_candidates_for_type(
    SemanticContext & ctx,
    const TypePtr & type,
    const string & name,
    vector<FunctionBinding *> * functions_out,
    vector<FunctionTemplateDecl *> * templates_out)
{
  AssociatedFriendLookupCacheKey key;
  const bool have_key = make_associated_friend_lookup_cache_key(type, name, key);
  if(have_key) {
    AssociatedFriendLookupCache & cache = associated_friend_lookup_cache();
    AssociatedFriendLookupCache::const_iterator cached = cache.find(key);
    if(cached != cache.end()) {
      if(functions_out) {
        append_unique_functions(*functions_out, cached->second.functions);
      }
      if(templates_out) {
        append_unique_function_templates(*templates_out, cached->second.templates);
      }
      return;
    }
  }

  TypePtr base = have_key ? key.type : associated_namespace_scope_cache_key_type(type);
  if(!base) {
    return;
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(!info) {
    info = ctx.class_info_for_type(base);
  }
  if(!info) {
    return;
  }

  AssociatedFriendLookupCacheEntry entry;
  set<const ClassInfo *> visited_functions;
  collect_associated_friend_functions_for_class(
      info, name, visited_functions, entry.functions);
  set<const ClassInfo *> visited_templates;
  collect_associated_friend_function_templates_for_class(
      info, name, visited_templates, entry.templates);

  if(functions_out) {
    append_unique_functions(*functions_out, entry.functions);
  }
  if(templates_out) {
    append_unique_function_templates(*templates_out, entry.templates);
  }

  if(have_key) {
    set<const ClassInfo *> visited_cacheable;
    if(associated_friend_lookup_cacheable_class(info, visited_cacheable)) {
      AssociatedFriendLookupCache & cache = associated_friend_lookup_cache();
      if(cache.size() > 262144) {
        cache.clear();
      }
      cache[key] = entry;
    }
  }
}

}  // namespace

void lookup_associated_friend_functions_for_type(SemanticContext & ctx,
                                                 const TypePtr & type,
                                                 const string & name,
                                                 vector<FunctionBinding *> & out)
{
  lookup_associated_friend_candidates_for_type(ctx, type, name, &out, nullptr);
}

void lookup_associated_friend_function_templates_for_type(
    SemanticContext & ctx,
    const TypePtr & type,
    const string & name,
    vector<FunctionTemplateDecl *> & out)
{
  lookup_associated_friend_candidates_for_type(ctx, type, name, nullptr, &out);
}

ClassTemplateDecl * lookup_class_template(SemanticContext & ctx,
                                          Scope & scope,
                                          const QualifiedName & qualified)
{
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    return lookup_unqualified_decl_with_entity_equivalence<ClassTemplateDecl>(
        scope,
        qualified.name,
        [&ctx](Scope & target, const string & lookup_name) -> ClassTemplateDecl *
        {
          return lookup_class_template_in_scope_or_inherited_members(ctx,
                                                                     target,
                                                                     lookup_name);
        },
        [](ClassTemplateDecl * lhs, ClassTemplateDecl * rhs) -> bool
        {
          return same_inline_namespace_class_template_entity(lhs, rhs);
        });
  }

  return lookup_qualified_class_or_namespace_generic<ClassTemplateDecl *>(
      ctx, scope, qualified,
      [](Scope & target, const string & lookup_name) -> ClassTemplateDecl *
      {
        return lookup_qualified_decl_with_using_directives<ClassTemplateDecl>(
            target,
            lookup_name,
            [](Scope & direct_scope, const string & direct_name) -> ClassTemplateDecl *
            {
              return lookup_class_template_in_direct_or_inline_scope(direct_scope,
                                                                     direct_name);
            },
            [](ClassTemplateDecl * lhs, ClassTemplateDecl * rhs) -> bool
            {
              return same_inline_namespace_class_template_entity(lhs, rhs);
            });
      });
}

ClassTemplateDecl * lookup_class_template(SemanticContext & ctx,
                                          Scope & scope,
                                          const string & name)
{
  return lookup_unqualified_decl_with_entity_equivalence<ClassTemplateDecl>(
      scope, name,
      [&ctx](Scope & target, const string & lookup_name) -> ClassTemplateDecl *
      {
        return lookup_class_template_in_scope_or_inherited_members(ctx,
                                                                   target,
                                                                   lookup_name);
      },
      [](ClassTemplateDecl * lhs, ClassTemplateDecl * rhs) -> bool
      {
        return same_inline_namespace_class_template_entity(lhs, rhs);
      });
}

ClassTemplateDecl * lookup_unqualified_class_template(Scope & scope,
                                                      const string & name)
{
  return lookup_unqualified_decl_with_entity_equivalence<ClassTemplateDecl>(
      scope, name,
      [](Scope & target, const string & lookup_name) -> ClassTemplateDecl *
      {
        map<string, ClassTemplateDecl *>::iterator found =
            target.class_templates.find(lookup_name);
        return found == target.class_templates.end() ? nullptr : found->second;
      },
      [](ClassTemplateDecl * lhs, ClassTemplateDecl * rhs) -> bool
      {
        return same_inline_namespace_class_template_entity(lhs, rhs);
      });
}

AliasTemplateDecl * lookup_alias_template(SemanticContext & ctx,
                                          Scope & scope,
                                          const QualifiedName & qualified)
{
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    return lookup_unqualified_decl_with_entity_equivalence<AliasTemplateDecl>(
        scope,
        qualified.name,
        [&ctx](Scope & target, const string & lookup_name) -> AliasTemplateDecl *
        {
          return lookup_alias_template_in_scope_or_inherited_members(ctx,
                                                                     target,
                                                                     lookup_name);
        },
        [](AliasTemplateDecl * lhs, AliasTemplateDecl * rhs) -> bool
        {
          return same_inline_namespace_alias_template_entity(lhs, rhs);
        });
  }

  return lookup_qualified_class_or_namespace_generic<AliasTemplateDecl *>(
      ctx, scope, qualified,
      [](Scope & target, const string & lookup_name) -> AliasTemplateDecl *
      {
        return lookup_qualified_decl_with_using_directives<AliasTemplateDecl>(
            target,
            lookup_name,
            [](Scope & direct_scope, const string & direct_name) -> AliasTemplateDecl *
            {
              return lookup_alias_template_in_direct_or_inline_scope(direct_scope,
                                                                     direct_name);
            },
            [](AliasTemplateDecl * lhs, AliasTemplateDecl * rhs) -> bool
            {
              return same_inline_namespace_alias_template_entity(lhs, rhs);
            });
      });
}

AliasTemplateDecl * lookup_alias_template(SemanticContext & ctx,
                                          Scope & scope,
                                          const string & name)
{
  return lookup_unqualified_decl_with_entity_equivalence<AliasTemplateDecl>(
      scope, name,
      [&ctx](Scope & target, const string & lookup_name) -> AliasTemplateDecl *
      {
        return lookup_alias_template_in_scope_or_inherited_members(ctx,
                                                                   target,
                                                                   lookup_name);
      },
      [](AliasTemplateDecl * lhs, AliasTemplateDecl * rhs) -> bool
      {
        return same_inline_namespace_alias_template_entity(lhs, rhs);
      });
}

AliasTemplateDecl * lookup_unqualified_alias_template(Scope & scope,
                                                      const string & name)
{
  return lookup_unqualified_decl_with_entity_equivalence<AliasTemplateDecl>(
      scope, name,
      [](Scope & target, const string & lookup_name) -> AliasTemplateDecl *
      {
        map<string, AliasTemplateDecl *>::iterator found =
            target.alias_templates.find(lookup_name);
        return found == target.alias_templates.end() ? nullptr : found->second;
      },
      [](AliasTemplateDecl * lhs, AliasTemplateDecl * rhs) -> bool
      {
        return same_inline_namespace_alias_template_entity(lhs, rhs);
      });
}

vector<FunctionTemplateDecl *> lookup_function_templates(SemanticContext & ctx,
                                                         Scope & scope,
                                                         const string & name)
{
  vector<FunctionTemplateDecl *> out;
  collect_function_templates(ctx, scope, name, out);
  return out;
}

void collect_function_templates(SemanticContext & ctx,
                                Scope & scope,
                                const QualifiedName & qualified,
                                vector<FunctionTemplateDecl *> & out)
{
  out.clear();
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    collect_function_templates(ctx, scope, qualified.name, out);
    return;
  }

  Scope * target = resolve_qualified_scope_for_class_or_namespace(ctx, scope, qualified);
  if(!target) {
    return;
  }
  if(target->class_info) {
    MemberFunctionTemplateLookupResult result =
        lookup_visible_member_function_templates(*target->class_info, qualified.name);
    append_unique_function_templates(out, result.templates);
    if(!out.empty()) {
      return;
    }
    collect_direct_function_templates(*target, qualified.name, out);
    return;
  }
  lookup_function_templates_in_scopes(vector<Scope *>(1, target), qualified.name, out);
}

void collect_function_templates(SemanticContext &,
                                Scope & scope,
                                const string & name,
                                vector<FunctionTemplateDecl *> & out)
{
  out.clear();

  for(Scope * current = &scope; current; current = current->parent) {
    out.clear();
    collect_direct_function_templates(*current, name, out);
    if(out.empty() && current->class_info) {
      MemberFunctionTemplateLookupResult result =
          lookup_visible_member_function_templates(*current->class_info, name);
      append_unique_function_templates(out, result.templates);
    }
    const bool has_lexical_class =
        !current->class_info && current->function && current->function->lexical_access_class;
    const bool lexical_only =
        has_lexical_class &&
        (!current->function->is_method ||
         current->function->owner_class != current->function->lexical_access_class);
    ClassInfo * lexical_class = current->class_info;
    if(!lexical_class && has_lexical_class) {
      lexical_class = current->function->lexical_access_class;
    }
    if(out.empty() && lexical_class) {
      MemberFunctionTemplateLookupResult result =
          lookup_visible_member_function_templates(*lexical_class, name);
      append_unique_function_templates(out, result.templates);
      if(lexical_only) {
        out.erase(remove_if(out.begin(),
                            out.end(),
                            [](FunctionTemplateDecl * decl)
                            {
                              return decl && !decl->is_static_member;
                            }),
                  out.end());
      }
    }
    set<const Scope *> visited;
    lookup_function_templates_from_using_directives(*current, name, visited, out);
    if(!out.empty()) {
      return;
    }
  }
}

VariableTemplateDecl * lookup_variable_template(SemanticContext & ctx,
                                                Scope & scope,
                                                const QualifiedName & qualified)
{
  const auto lookup_in_scope =
      [&ctx](Scope & target, const string & lookup_name) -> VariableTemplateDecl *
      {
        if(target.class_info) {
          MemberVariableTemplateLookupResult member =
              lookup_member_variable_template(ctx, *target.class_info, lookup_name);
          if(member.variable_template) {
            return member.variable_template;
          }
        }
        map<string, VariableTemplateDecl *>::iterator found =
            target.variable_templates.find(lookup_name);
        return found == target.variable_templates.end() ? nullptr : found->second;
      };

  if(qualified.rooted || !qualified.qualifiers.empty()) {
    return lookup_qualified_class_or_namespace_generic<VariableTemplateDecl *>(
        ctx, scope, qualified, lookup_in_scope);
  }

  return lookup_unqualified_with_present<VariableTemplateDecl *>(
      scope, qualified.name,
      lookup_in_scope,
      [](VariableTemplateDecl * result) -> bool
      {
        return result != nullptr;
      });
}

VariableTemplateDecl * lookup_variable_template(SemanticContext & ctx,
                                                Scope & scope,
                                                const string & name)
{
  const auto lookup_in_scope =
      [&ctx](Scope & target, const string & lookup_name) -> VariableTemplateDecl *
      {
        if(target.class_info) {
          MemberVariableTemplateLookupResult member =
              lookup_member_variable_template(ctx, *target.class_info, lookup_name);
          if(member.variable_template) {
            return member.variable_template;
          }
        }
        map<string, VariableTemplateDecl *>::iterator found =
            target.variable_templates.find(lookup_name);
        return found == target.variable_templates.end() ? nullptr : found->second;
      };

  return lookup_unqualified_with_present<VariableTemplateDecl *>(
      scope, name,
      lookup_in_scope,
      [](VariableTemplateDecl * result) -> bool
      {
        return result != nullptr;
      });
}

Scope * resolve_qualified_scope_for_class_or_namespace(SemanticContext & ctx,
                                                       Scope & scope,
                                                       const QualifiedName & qualified,
                                                       bool allow_dependent_class_qualifiers)
{
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    return qualified.name.empty() ?
        nullptr :
        resolve_type_qualifier_scope(ctx,
                                     scope,
                                     scope,
                                     qualified.name,
                                     allow_dependent_class_qualifiers);
  }
  Scope * current = qualified.rooted ? root_scope(scope) :
      resolve_type_qualifier_scope(ctx,
                                   scope,
                                   scope,
                                   qualified.qualifiers[0],
                                   allow_dependent_class_qualifiers);
  size_t next = qualified.rooted ? 0 : 1;
  if(!current) {
    return nullptr;
  }
  while(next < qualified.qualifiers.size()) {
    current = resolve_nested_type_qualifier_scope(
        ctx,
        *current,
        scope,
        qualified.qualifiers[next],
        allow_dependent_class_qualifiers);
    if(!current) {
      return nullptr;
    }
    ++next;
  }
  return current;
}

string qualified_value_qualifier_text(const QualifiedName & qualified_name)
{
  string out;
  if(qualified_name.rooted) {
    out += "::";
  }
  for(size_t i = 0; i < qualified_name.qualifiers.size(); ++i) {
    if(i != 0) {
      out += "::";
    }
    out += qualified_name.qualifiers[i];
  }
  return out;
}

bool node_has_structured_qualifier_syntax(const CppAstNode & node)
{
  return !node.qualifier_template_id_syntaxes.empty() ||
         !node.qualifier_type_syntaxes.empty();
}

const ValueBinding * lookup_value_binding_in_type_scope(SemanticContext & ctx,
                                                        Scope & scope,
                                                        const QualifiedName & qualified,
                                                        const TypePtr & qualifier_type)
{
  if(!qualifier_type) {
    return nullptr;
  }

  if(is_named_enum_type(ctx, qualifier_type)) {
    Scope * enum_scope = ctx.scope_for_type(qualifier_type);
    if(enum_scope) {
      map<string, ValueBinding>::const_iterator direct =
          enum_scope->values.find(qualified.name);
      if(direct != enum_scope->values.end()) {
        return &direct->second;
      }
    }
    for(Scope * current = &scope; current; current = current->parent) {
      map<string, ValueBinding>::const_iterator found =
          current->values.find(qualified.name);
      if(found != current->values.end() &&
         type_equals(strip_top_level_cv(found->second.type),
                     strip_top_level_cv(qualifier_type))) {
        return &found->second;
      }
    }
    return nullptr;
  }

  ClassInfo * qualifier_info = ctx.class_info_for_type(qualifier_type);
  if(!qualifier_info) {
    qualifier_info = ctx.complete_class_type(qualifier_type);
  }
  if(!qualifier_info) {
    return nullptr;
  }

  if(qualifier_info->member_scope &&
     ctx.scope_has_template_placeholders(*qualifier_info->member_scope) &&
     !is_concrete_class_template_qualifier(ctx, qualifier_info)) {
    ensure_class_reference_members_if_needed(ctx, scope, *qualifier_info);
  }
  if(qualifier_info->member_scope &&
     !qualifier_info->complete &&
     !qualifier_info->full_member_collection_in_progress &&
     is_concrete_class_template_qualifier(ctx, qualifier_info)) {
    if(ClassInfo * completed = ctx.complete_class_type(qualifier_type)) {
      qualifier_info = completed;
    }
  }
  if(!qualifier_info->member_scope) {
    qualifier_info = ctx.complete_class_type(qualifier_type);
  }
  if(!qualifier_info || !qualifier_info->member_scope) {
    return nullptr;
  }

  Scope * target = qualifier_info->member_scope.get();
  map<string, ValueBinding>::const_iterator found = target->values.find(qualified.name);
  if(found != target->values.end()) {
    return &found->second;
  }

  MemberValueLookupResult member = lookup_member_value(*qualifier_info, qualified.name);
  if(member.binding) {
    return member.binding;
  }

  if(!qualifier_info->complete &&
     (is_concrete_class_template_qualifier(ctx, qualifier_info) ||
      !ctx.scope_has_template_placeholders(*qualifier_info->member_scope))) {
    ClassInfo * completed = ctx.complete_class_type(qualifier_info->type);
    if(completed && completed->member_scope) {
      target = completed->member_scope.get();
      found = target->values.find(qualified.name);
      if(found != target->values.end()) {
        return &found->second;
      }
      member = lookup_member_value(*completed, qualified.name);
      if(member.binding) {
        return member.binding;
      }
    }
  }
  return nullptr;
}

const ValueBinding * lookup_qualified_value_binding(SemanticContext & ctx,
                                                    Scope & scope,
                                                    const QualifiedName & qualified)
{
  const string qualifier_name = qualified_value_qualifier_text(qualified);

  Scope * target = resolve_qualified_scope_for_class_or_namespace(ctx, scope, qualified);
  if(!target) {
    if(!qualifier_name.empty()) {
      TypePtr qualifier_type = ctx.lookup_type(scope, qualifier_name, false);
      if(qualifier_type && is_named_enum_type(ctx, qualifier_type)) {
        Scope * enum_scope = ctx.scope_for_type(qualifier_type);
        if(enum_scope) {
          map<string, ValueBinding>::const_iterator direct =
              enum_scope->values.find(qualified.name);
          if(direct != enum_scope->values.end()) {
            return &direct->second;
          }
        }
        for(Scope * current = &scope; current; current = current->parent) {
          map<string, ValueBinding>::const_iterator found = current->values.find(qualified.name);
          if(found != current->values.end() &&
             type_equals(strip_top_level_cv(found->second.type),
                         strip_top_level_cv(qualifier_type))) {
            return &found->second;
          }
        }
      }
    }
    return nullptr;
  }
  if(!target->class_info) {
    const ValueBinding * direct = lookup_direct_value(*target, qualified.name);
    set<const Scope *> visited;
    ValueLookupFromUsingDirectivesResult imported =
        lookup_value_from_using_directives(*target, qualified.name, visited);
    if(imported.ambiguous) {
      return nullptr;
    }
    if(direct && imported.binding &&
       !same_value_binding_entity(direct, imported.binding)) {
      return nullptr;
    }
    if(direct) {
      return direct;
    }
    if(imported.binding) {
      return imported.binding;
    }
  }
  if(target->class_info &&
     ctx.scope_has_template_placeholders(*target->class_info->member_scope)) {
    const ClassInfo * qualifier_info = target->class_info;
    const bool concrete_qualifier =
        !qualified.qualifiers.empty() &&
        is_concrete_class_template_qualifier(ctx, qualifier_info);
    if(!concrete_qualifier) {
      if(qualifier_name.empty()) {
        return nullptr;
      }
      TypePtr qualifier_type = ctx.lookup_type(scope, qualifier_name, true);
      ClassInfo * qualifier_info = qualifier_type ? ctx.class_info_for_type(qualifier_type) : nullptr;
      if(!qualifier_info) {
        return nullptr;
      }
      ensure_class_reference_members_if_needed(ctx, scope, *qualifier_info);
      target = qualifier_info->member_scope.get();
    }
  }
  if(target->class_info &&
     !target->class_info->complete &&
     !target->class_info->full_member_collection_in_progress &&
     is_concrete_class_template_qualifier(ctx, target->class_info)) {
    if(ClassInfo * completed = ctx.complete_class_type(target->class_info->type)) {
      target = completed->member_scope.get();
    }
  }
  map<string, ValueBinding>::const_iterator found = target->values.find(qualified.name);
  if(found != target->values.end()) {
    return &found->second;
  }
  if(target->class_info) {
    MemberValueLookupResult member = lookup_member_value(*target->class_info, qualified.name);
    if(member.binding) {
      return member.binding;
    }
    if(!target->class_info->complete &&
       (is_concrete_class_template_qualifier(ctx, target->class_info) ||
        !ctx.scope_has_template_placeholders(*target->class_info->member_scope))) {
      ClassInfo * completed = ctx.complete_class_type(target->class_info->type);
      if(completed) {
        target = completed->member_scope.get();
        found = target->values.find(qualified.name);
        if(found != target->values.end()) {
          return &found->second;
        }
        member = lookup_member_value(*completed, qualified.name);
        if(member.binding) {
          return member.binding;
        }
      }
    }
  }
  if(!qualifier_name.empty()) {
    TypePtr qualifier_type = ctx.lookup_type(scope,
                                            qualifier_name,
                                            ctx.text_mentions_template_placeholders(scope,
                                                                                   qualifier_name));
    if(qualifier_type) {
      ClassInfo * qualifier_info = ctx.class_info_for_type(qualifier_type);
      if(qualifier_info &&
         ctx.scope_has_template_placeholders(*qualifier_info->member_scope) &&
         !is_concrete_class_template_qualifier(ctx, qualifier_info)) {
        ensure_class_reference_members_if_needed(ctx, scope, *qualifier_info);
        target = qualifier_info->member_scope.get();
        found = target->values.find(qualified.name);
        if(found != target->values.end()) {
          return &found->second;
        }
        MemberValueLookupResult member = lookup_member_value(*qualifier_info, qualified.name);
        if(member.binding) {
          return member.binding;
        }
        return nullptr;
      }
      ClassInfo * completed = ctx.complete_class_type(qualifier_type);
      if(completed) {
        target = completed->member_scope.get();
        found = target->values.find(qualified.name);
        if(found != target->values.end()) {
          return &found->second;
        }
        MemberValueLookupResult member = lookup_member_value(*completed, qualified.name);
        if(member.binding) {
          return member.binding;
        }
      }
    }
  }
  return nullptr;
}

const ValueBinding * lookup_qualified_value_binding_node(SemanticContext & ctx,
                                                         Scope & scope,
                                                         const QualifiedName & qualified,
                                                         const CppAstNode & node)
{
  if(!node_has_structured_qualifier_syntax(node)) {
    return lookup_qualified_value_binding(ctx, scope, qualified);
  }

  const string qualifier_name = qualified_value_qualifier_text(qualified);
  if(qualifier_name.empty()) {
    return lookup_qualified_value_binding(ctx, scope, qualified);
  }

  if(!qualified.qualifiers.empty()) {
    const size_t final_qualifier_index = qualified.qualifiers.size() - 1;
    if(const CppAstNode * qualifier =
           cppast_qualifier_type_syntax(node, final_qualifier_index)) {
      TypePtr qualifier_type;
      if(ctx.parse_decltype_specifier(scope, *qualifier, qualifier_type)) {
        return lookup_value_binding_in_type_scope(ctx, scope, qualified, qualifier_type);
      }
    }
  }

  TypePtr qualifier_type = ctx.lookup_type_node(scope, node, qualifier_name, false);
  return lookup_value_binding_in_type_scope(ctx, scope, qualified, qualifier_type);
}

}  // namespace semantic_lookup
