#include "template_witness_renderer.h"

#include "witness_text.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using std::map;
using std::pair;
using std::set;
using std::string;
using std::tuple;
using std::vector;

string trim_space(const string & text)
{
  string::size_type begin = 0;
  while(begin < text.size() &&
        std::isspace(static_cast<unsigned char>(text[begin]))) {
    ++begin;
  }
  string::size_type end = text.size();
  while(end > begin &&
        std::isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }
  return text.substr(begin, end - begin);
}

string strip_simple_elaborated_prefix(const string & text)
{
  const string prefixes[] = {"typename ", "class ", "struct ", "union "};
  string out = trim_space(text);
  bool changed = true;
  while(changed) {
    changed = false;
    for(size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
      if(out.compare(0, prefixes[i].size(), prefixes[i]) == 0) {
        out = trim_space(out.substr(prefixes[i].size()));
        changed = true;
        break;
      }
    }
  }
  return out;
}

void replace_all(string & text,
                 const string & needle,
                 const string & replacement)
{
  if(needle.empty()) {
    return;
  }
  string::size_type pos = 0;
  while((pos = text.find(needle, pos)) != string::npos) {
    text.replace(pos, needle.size(), replacement);
    pos += replacement.size();
  }
}

string normalize_compact_type_layout(const string & text)
{
  static const std::regex pointer_suffix_regex("([A-Za-z_0-9>\\)])\\*");
  static const std::regex rvalue_ref_suffix_regex("([A-Za-z_0-9>\\)])&&");
  static const std::regex lvalue_ref_suffix_regex("([A-Za-z_0-9>\\)])&");
  static const std::regex call_paren_regex("([A-Za-z_0-9>])\\(");
  static const std::regex pointer_const_regex("\\*\\s+const\\b");
  static const std::regex pointer_volatile_regex("\\*\\s+volatile\\b");
  static const std::regex pointer_rvalue_ref_regex("\\*\\s+&&");
  static const std::regex pointer_lvalue_ref_regex("\\*\\s+&");
  string out = text;
  out = std::regex_replace(out, pointer_suffix_regex, "$1 *");
  out = std::regex_replace(out, rvalue_ref_suffix_regex, "$1 &&");
  out = std::regex_replace(out, lvalue_ref_suffix_regex, "$1 &");
  out = std::regex_replace(out, call_paren_regex, "$1 (");
  replace_all(out, "operator (", "operator(");
  replace_all(out, "operator &&", "operator&&");
  replace_all(out, "operator &", "operator&");
  replace_all(out, "operator *", "operator*");
  out = std::regex_replace(out, pointer_const_regex, "*const");
  out = std::regex_replace(out, pointer_volatile_regex, "*volatile");
  out = std::regex_replace(out, pointer_rvalue_ref_regex, "*&&");
  out = std::regex_replace(out, pointer_lvalue_ref_regex, "*&");
  return out;
}

string normalize_template_argument_separator_layout(const string & text)
{
  string out;
  out.reserve(text.size());
  int angle_depth = 0;
  bool in_string = false;
  bool in_char = false;
  bool escaped = false;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(in_string || in_char) {
      out += ch;
      if(escaped) {
        escaped = false;
      } else if(ch == '\\') {
        escaped = true;
      } else if(in_string && ch == '"') {
        in_string = false;
      } else if(in_char && ch == '\'') {
        in_char = false;
      }
      continue;
    }
    if(ch == '"') {
      in_string = true;
      out += ch;
      continue;
    }
    if(ch == '\'') {
      in_char = true;
      out += ch;
      continue;
    }
    if(ch == '<') {
      ++angle_depth;
      out += ch;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      out += ch;
      continue;
    }
    if(ch == ',' && angle_depth > 0) {
      out += ", ";
      while(i + 1 < text.size() &&
            std::isspace(static_cast<unsigned char>(text[i + 1]))) {
        ++i;
      }
      continue;
    }
    out += ch;
  }
  return out;
}

string normalize_source_event_type_spellings(const string & text)
{
  static const std::regex signed_long_long_int_regex("\\bsigned long long int\\b");
  static const std::regex signed_long_int_regex("\\bsigned long int\\b");
  static const std::regex signed_short_int_regex("\\bsigned short int\\b");
  static const std::regex signed_long_long_regex("\\bsigned long long\\b");
  static const std::regex signed_long_regex("\\bsigned long\\b");
  static const std::regex signed_short_regex("\\bsigned short\\b");
  static const std::regex signed_int_regex("\\bsigned int\\b");
  static const std::regex signed_regex("\\bsigned\\b(?!\\s+char\\b)");
  static const std::regex integer_suffix_regex("\\b([0-9]+)[uUlL]+\\b");
  static const std::regex double_pointer_space_regex("\\*\\s+\\*");
  static const std::regex member_function_scope_regex(
      "::([A-Za-z_~][A-Za-z0-9_~]*) \\(");
  string out = text;
  replace_all(out, "unsigned long long int", "unsigned long long");
  replace_all(out, "unsigned long int", "unsigned long");
  replace_all(out, "long long int", "long long");
  replace_all(out, "long int", "long");
  replace_all(out, "short int", "short");
  replace_all(out, "unsigned int", "unsigned");
  out = std::regex_replace(out, signed_long_long_int_regex, "long long");
  out = std::regex_replace(out, signed_long_int_regex, "long");
  out = std::regex_replace(out, signed_short_int_regex, "short");
  out = std::regex_replace(out, signed_long_long_regex, "long long");
  out = std::regex_replace(out, signed_long_regex, "long");
  out = std::regex_replace(out, signed_short_regex, "short");
  out = std::regex_replace(out, signed_int_regex, "signed");
  out = std::regex_replace(out, signed_regex, "int");
  out = std::regex_replace(out, integer_suffix_regex, "$1");
  out = normalize_template_argument_separator_layout(out);
  while(true) {
    const string collapsed =
        std::regex_replace(out, double_pointer_space_regex, "**");
    if(collapsed == out) {
      break;
    }
    out = collapsed;
  }
  out = normalize_compact_type_layout(out);
  return std::regex_replace(out, member_function_scope_regex, "::$1(");
}

string normalize_string_literal_array_witness_text(const string & text)
{
  static const std::regex const_char_array_regex(
      "\\bconst\\s+(char|wchar_t|char16_t|char32_t)\\s*\\[");
  return std::regex_replace(text, const_char_array_regex, "$1[");
}

string collapse_duplicate_owner_prefix(const string & entity)
{
  const string::size_type member_pos = entity.rfind("::");
  if(member_pos == string::npos) {
    return entity;
  }

  const string owner = entity.substr(0, member_pos);
  const string member = entity.substr(member_pos + 2);
  for(string::size_type split = owner.find("::");
      split != string::npos;
      split = owner.find("::", split + 2)) {
    const string owner_prefix = owner.substr(0, split);
    const string owner_suffix = owner.substr(split + 2);
    if(owner_prefix == owner_suffix) {
      return owner_suffix + "::" + member;
    }
  }
  return entity;
}

string normalize_source_event_entity_text(const string & entity)
{
  return witness_text::normalize_anonymous_namespace_segments(
      normalize_string_literal_array_witness_text(
          normalize_source_event_type_spellings(
              collapse_duplicate_owner_prefix(entity))));
}

vector<string> split_top_level(const string & text, char separator)
{
  vector<string> out;
  string current;
  int depth_angle = 0;
  int depth_brace = 0;
  int depth_paren = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++depth_angle;
    } else if(ch == '>') {
      depth_angle = std::max(depth_angle - 1, 0);
    } else if(ch == '{') {
      ++depth_brace;
    } else if(ch == '}') {
      depth_brace = std::max(depth_brace - 1, 0);
    } else if(ch == '(') {
      ++depth_paren;
    } else if(ch == ')') {
      depth_paren = std::max(depth_paren - 1, 0);
    }
    if(ch == separator && depth_angle == 0 && depth_brace == 0 &&
       depth_paren == 0) {
      out.push_back(trim_space(current));
      current.clear();
      continue;
    }
    current.push_back(ch);
  }
  out.push_back(trim_space(current));
  return out;
}

int find_matching_paren(const string & text, size_t open_pos)
{
  int depth = 0;
  for(size_t i = open_pos; i < text.size(); ++i) {
    if(text[i] == '(') {
      ++depth;
    } else if(text[i] == ')') {
      --depth;
      if(depth == 0) {
        return static_cast<int>(i);
      }
    }
  }
  return -1;
}

string normalize_witness_path(const string & path)
{
  static map<string, string> cache;
  map<string, string>::const_iterator cached = cache.find(path);
  if(cached != cache.end()) {
    return cached->second;
  }
  if(path.empty()) {
    cache[path] = string();
    return string();
  }
  string value = path;
  std::replace(value.begin(), value.end(), '\\', '/');
  const string libcxx_marker = "/include/c++/v1/";
  const string::size_type libcxx_pos = value.find(libcxx_marker);
  if(libcxx_pos != string::npos) {
    cache[path] = "libc++/" + value.substr(libcxx_pos + libcxx_marker.size());
    return cache[path];
  }
  static const std::regex pa_regex("(^|/)pa[0-9]+/((tests|course)/.*)$");
  std::smatch match;
  if(std::regex_search(value, match, pa_regex)) {
    cache[path] = match[2].str();
    return cache[path];
  }
  cache[path] = value;
  return value;
}

string move_trailing_cv_to_prefix(const string & text)
{
  string value = trim_space(text);
  bool changed = true;
  while(changed) {
    changed = false;
    static const std::regex const_paren_suffix("^(.*) const (\\([^)]*\\).*)$");
    static const std::regex const_ref_suffix("^(.*) const (&&|&|\\*)$");
    static const std::regex volatile_paren_suffix("^(.*) volatile (\\([^)]*\\).*)$");
    static const std::regex volatile_ref_suffix("^(.*) volatile (&&|&|\\*)$");
    std::smatch match;
    if(std::regex_match(value, match, const_paren_suffix) ||
       std::regex_match(value, match, const_ref_suffix)) {
      value = "const " + trim_space(match[1].str()) + " " +
          trim_space(match[2].str());
      changed = true;
    }
    const string::size_type const_array_pos = value.find(" const [");
    if(const_array_pos != string::npos) {
      value = "const " + trim_space(value.substr(0, const_array_pos)) + " " +
          trim_space(value.substr(const_array_pos + 7));
      changed = true;
    }
    if(std::regex_match(value, match, volatile_paren_suffix) ||
       std::regex_match(value, match, volatile_ref_suffix)) {
      value = "volatile " + trim_space(match[1].str()) + " " +
          trim_space(match[2].str());
      changed = true;
    }
    const string::size_type volatile_array_pos = value.find(" volatile [");
    if(volatile_array_pos != string::npos) {
      value = "volatile " + trim_space(value.substr(0, volatile_array_pos)) + " " +
          trim_space(value.substr(volatile_array_pos + 10));
      changed = true;
    }
    if(value.size() >= 6 &&
       value.compare(value.size() - 6, 6, " const") == 0 &&
       value.compare(0, 6, "const ") != 0) {
      value = "const " + trim_space(value.substr(0, value.size() - 6));
      changed = true;
    }
    if(value.size() >= 9 &&
       value.compare(value.size() - 9, 9, " volatile") == 0 &&
       value.compare(0, 9, "volatile ") != 0) {
      value = "volatile " + trim_space(value.substr(0, value.size() - 9));
      changed = true;
    }
  }
  return trim_space(value);
}

string attach_type_suffix(const string & base, const string & suffix)
{
  const string normalized_base = trim_space(base);
  const string::size_type array_split = normalized_base.find(" [");
  if(array_split != string::npos) {
    return normalized_base.substr(0, array_split) + " (" + suffix + ")" +
        normalized_base.substr(array_split + 1);
  }
  const string::size_type function_split = normalized_base.find(" (");
  if(function_split != string::npos && !normalized_base.empty() &&
     normalized_base[normalized_base.size() - 1] == ')') {
    return normalized_base.substr(0, function_split) + " (" + suffix + ")" +
        normalized_base.substr(function_split + 1);
  }
  return normalized_base + " " + suffix;
}

string normalize_student_text(const string & text);

string normalize_array_text(const string & text)
{
  const string prefix = "array of ";
  const size_t count_begin = prefix.size();
  const size_t count_end = text.find(' ', count_begin);
  if(count_end == string::npos) {
    return trim_space(text);
  }
  const string count = trim_space(text.substr(count_begin, count_end - count_begin));
  const string element = trim_space(text.substr(count_end + 1));
  if(count.empty() || element.empty()) {
    return trim_space(text);
  }
  return normalize_student_text(element) + " [" + count + "]";
}

string normalize_function_text(const string & text)
{
  const string prefix = "function of (";
  if(text.compare(0, prefix.size(), prefix) != 0) {
    return trim_space(text);
  }
  const size_t open_pos = text.find('(');
  if(open_pos == string::npos) {
    return trim_space(text);
  }
  const int close_pos = find_matching_paren(text, open_pos);
  if(close_pos < 0) {
    return trim_space(text);
  }
  const size_t returning_pos = text.find(" returning ", static_cast<size_t>(close_pos) + 1);
  if(returning_pos == string::npos) {
    return trim_space(text);
  }
  const string params_body = text.substr(open_pos + 1,
                                         returning_pos - open_pos - 10);
  const string return_body = trim_space(text.substr(returning_pos + 11));
  vector<string> params;
  const vector<string> raw_params = split_top_level(params_body, ',');
  for(size_t i = 0; i < raw_params.size(); ++i) {
    if(!trim_space(raw_params[i]).empty()) {
      params.push_back(normalize_student_text(raw_params[i]));
    }
  }
  string out = move_trailing_cv_to_prefix(normalize_student_text(return_body)) +
      " (";
  for(size_t i = 0; i < params.size(); ++i) {
    if(i) {
      out += ", ";
    }
    out += params[i];
  }
  out += ")";
  return out;
}

string normalize_student_text(const string & text)
{
  string value = trim_space(text);
  if(value.compare(0, 20, "lvalue-reference to ") == 0) {
    return attach_type_suffix(
        normalize_student_text(value.substr(20)),
        "&");
  }
  if(value.compare(0, 20, "rvalue-reference to ") == 0) {
    return attach_type_suffix(
        normalize_student_text(value.substr(20)),
        "&&");
  }
  if(value.compare(0, 11, "pointer to ") == 0) {
    return attach_type_suffix(
        normalize_student_text(value.substr(11)),
        "*");
  }
  if(value.compare(0, 18, "const function of (") == 0) {
    return normalize_function_text(value.substr(6));
  }
  if(value.compare(0, 21, "volatile function of (") == 0) {
    return normalize_function_text(value.substr(9));
  }
  if(value.compare(0, 9, "array of ") == 0) {
    return normalize_array_text(value);
  }
  if(value.compare(0, 13, "function of (") == 0) {
    return normalize_function_text(value);
  }
  static const std::regex classish_regex("\\b(struct|class|typename)\\s+");
  static const std::regex local_regex("__local_\\d+");
  static const std::regex top_owner_regex(
      "^[A-Z][A-Za-z0-9_]*::([A-Z][A-Za-z0-9_]*(<.*>)?)$");
  static const std::regex path_regex("((/[A-Za-z0-9_+.\\-]+)+:\\d+:\\d+)");
  static const std::regex libcxx_namespace_regex("std::__1::");
  value = std::regex_replace(value, libcxx_namespace_regex, "std::");
  value = std::regex_replace(value, classish_regex, "");
  value = std::regex_replace(value, local_regex, "");
  value = std::regex_replace(value, top_owner_regex, "$1");
  {
    string out;
    std::sregex_iterator it(value.begin(), value.end(), path_regex);
    std::sregex_iterator end;
    string::const_iterator cursor = value.begin();
    for(; it != end; ++it) {
      out.append(cursor, it->prefix().second);
      out += normalize_witness_path((*it)[1].str());
      cursor = it->suffix().first;
    }
    out.append(cursor, value.cend());
    value.swap(out);
  }
  return move_trailing_cv_to_prefix(value);
}

string strip_top_level_template_suffix(const string & text)
{
  const string value = trim_space(text);
  if(value.empty() || value[value.size() - 1] != '>') {
    return value;
  }
  int depth = 0;
  for(int i = static_cast<int>(value.size()) - 1; i >= 0; --i) {
    if(value[static_cast<size_t>(i)] == '>') {
      ++depth;
    } else if(value[static_cast<size_t>(i)] == '<') {
      --depth;
      if(depth == 0) {
        return trim_space(value.substr(0, static_cast<size_t>(i)));
      }
    }
  }
  return value;
}

using semantic_source_use::SourceBinding;
using semantic_source_use::SourceUseKind;
using SourceUseOwnership = semantic_source_use::SourceUseOwnership;
using SourceSelectionKind = semantic_source_use::SourceSelectionKind;

struct ParsedLocation
{
  int line = 0;
  int column = 0;
};

ParsedLocation parse_line_col(const string & location)
{
  ParsedLocation parsed;
  const string::size_type last_colon = location.rfind(':');
  if(last_colon == string::npos ||
     last_colon + 1 >= location.size()) {
    return parsed;
  }
  const string::size_type second_colon =
      last_colon == 0 ? string::npos : location.rfind(':', last_colon - 1);
  if(second_colon == string::npos ||
     second_colon + 1 >= last_colon) {
    return parsed;
  }
  for(string::size_type i = second_colon + 1; i < last_colon; ++i) {
    if(!std::isdigit(static_cast<unsigned char>(location[i]))) {
      return ParsedLocation();
    }
  }
  for(string::size_type i = last_colon + 1; i < location.size(); ++i) {
    if(!std::isdigit(static_cast<unsigned char>(location[i]))) {
      return ParsedLocation();
    }
  }
  parsed.line = std::atoi(location.c_str() + second_colon + 1);
  parsed.column = std::atoi(location.c_str() + last_colon + 1);
  return parsed;
}

struct RenderedSourceUse : semantic_source_use::SemanticSourceUse
{
  RenderedSourceUse() = default;

  explicit RenderedSourceUse(const semantic_source_use::SemanticSourceUse & use)
    : semantic_source_use::SemanticSourceUse(use)
  {
    template_name = normalize_source_event_entity_text(template_name);
    selected = normalize_source_event_entity_text(selected);
    for(size_t i = 0; i < drops.size(); ++i) {
      drops[i].candidate = normalize_source_event_entity_text(drops[i].candidate);
    }
  }

  size_t same_location_semantic_group_order = 0;
  int same_location_semantic_group_rank = 0;
};

const string & rendered_selected_decl_location(const RenderedSourceUse & event)
{
  return event.selected_decl_anchor_location.empty() ?
      event.selected_entity_decl_location :
      event.selected_decl_anchor_location;
}

string witness_selection_text(SourceSelectionKind selection,
                              SourceUseKind kind)
{
  switch(selection) {
  case SourceSelectionKind::None:
    return "";
  case SourceSelectionKind::Primary:
    return "primary";
  case SourceSelectionKind::PartialSpecialization:
    return "partial";
  case SourceSelectionKind::ExplicitSpecialization:
    return kind == SourceUseKind::FunctionCall ?
        "explicit_specialization" :
        "explicit";
  case SourceSelectionKind::Instantiation:
    return "instantiation";
  }
  return "";
}

string witness_selection_text(const RenderedSourceUse & event)
{
  return witness_selection_text(event.selection, event.kind);
}

SourceUseOwnership rendered_ownership_sort_key(SourceUseOwnership ownership)
{
  switch(ownership) {
  case SourceUseOwnership::NestedDerived:
    return SourceUseOwnership::NestedDerived;
  case SourceUseOwnership::Direct:
  case SourceUseOwnership::SourceOwned:
    return SourceUseOwnership::Direct;
  }
  return SourceUseOwnership::Direct;
}

string location_source_path(const string & location)
{
  const string::size_type last_colon = location.rfind(':');
  if(last_colon == string::npos || last_colon == 0) {
    return string();
  }
  const string::size_type second_colon = location.rfind(':', last_colon - 1);
  if(second_colon == string::npos) {
    return string();
  }
  return normalize_witness_path(location.substr(0, second_colon));
}

int binding_source_sort_rank(const RenderedSourceUse & event)
{
  int explicit_count = 0;
  int defaulted_count = 0;
  int deduced_count = 0;
  for(size_t i = 0; i < event.bindings.size(); ++i) {
    if(event.bindings[i].source == "explicit") {
      ++explicit_count;
    } else if(event.bindings[i].source == "defaulted") {
      ++defaulted_count;
    } else if(event.bindings[i].source == "deduced") {
      ++deduced_count;
    }
  }
  for(size_t i = 0; i < event.specialization_bindings.size(); ++i) {
    if(event.specialization_bindings[i].source == "explicit") {
      ++explicit_count;
    } else if(event.specialization_bindings[i].source == "defaulted") {
      ++defaulted_count;
    } else if(event.specialization_bindings[i].source == "deduced") {
      ++deduced_count;
    }
  }
  return -(explicit_count * 100 + defaulted_count * 10 - deduced_count);
}

int witness_event_semantic_sort_rank(const RenderedSourceUse & event)
{
  if(event.kind == SourceUseKind::FunctionCall &&
     event.same_location_semantic_group_rank == 1) {
    return 0;
  }
  if(event.kind == SourceUseKind::ClassUse) {
    return 1;
  }
  if(event.kind == SourceUseKind::AliasUse) {
    return 2;
  }
  if(event.kind == SourceUseKind::FunctionCall) {
    return 3;
  }
  return 4;
}

void group_member_calls_with_source_owner_class_uses(
    vector<RenderedSourceUse> & events)
{
  for(size_t class_index = 0; class_index < events.size(); ++class_index) {
    RenderedSourceUse & class_event = events[class_index];
    if(class_event.kind != SourceUseKind::ClassUse ||
       class_event.selection !=
           semantic_source_use::SourceSelectionKind::PartialSpecialization ||
       class_event.semantic_class_template_identity == nullptr) {
      continue;
    }
    for(size_t call_index = 0; call_index < events.size(); ++call_index) {
      RenderedSourceUse & call_event = events[call_index];
      if(call_event.kind != SourceUseKind::FunctionCall ||
         call_event.source_traversal_order == 0 ||
         call_event.location != class_event.location ||
         call_event.semantic_owner_class_template_identity !=
             class_event.semantic_class_template_identity ||
         call_event.semantic_owner_class_specialization_key !=
             class_event.semantic_class_specialization_key) {
        continue;
      }
      call_event.same_location_semantic_group_order =
          call_event.source_traversal_order;
      call_event.same_location_semantic_group_rank = 1;
      class_event.same_location_semantic_group_order =
          std::max(class_event.same_location_semantic_group_order,
                   call_event.source_traversal_order);
      class_event.same_location_semantic_group_rank = 2;
    }
  }
}

void group_materialized_class_uses_with_source_partial_class_uses(
    vector<RenderedSourceUse> & events)
{
  for(size_t partial_index = 0;
      partial_index < events.size();
      ++partial_index) {
    RenderedSourceUse & partial_event = events[partial_index];
    if(partial_event.kind != SourceUseKind::ClassUse ||
       partial_event.selection !=
           semantic_source_use::SourceSelectionKind::PartialSpecialization ||
       partial_event.source_traversal_order == 0 ||
       !partial_event.template_id_occurrence.present ||
       !partial_event.template_id_occurrence.source_spelled) {
      continue;
    }
    for(size_t materialized_index = 0;
        materialized_index < events.size();
        ++materialized_index) {
      RenderedSourceUse & materialized_event = events[materialized_index];
      if(materialized_event.kind != SourceUseKind::ClassUse ||
         materialized_event.role !=
             semantic_source_use::SourceUseRole::MaterializedTypeUse ||
         materialized_event.source_traversal_order != 0 ||
         materialized_event.location != partial_event.location) {
        continue;
      }
      materialized_event.same_location_semantic_group_order =
          std::max(materialized_event.same_location_semantic_group_order,
                   partial_event.source_traversal_order);
      materialized_event.same_location_semantic_group_rank = 1;
      partial_event.same_location_semantic_group_order =
          std::max(partial_event.same_location_semantic_group_order,
                   partial_event.source_traversal_order);
      partial_event.same_location_semantic_group_rank = 2;
    }
  }
}

size_t witness_event_source_sort_order(const RenderedSourceUse & event)
{
  const size_t source_order =
      event.same_location_semantic_group_order != 0 ?
          std::max(event.source_traversal_order,
                   event.same_location_semantic_group_order) :
          event.source_traversal_order;
  return source_order != 0 ? source_order : static_cast<size_t>(-1);
}

typedef tuple<string,
              int,
              int,
              int,
              int,
              size_t,
              int,
              int,
              SourceUseOwnership,
              string>
    WitnessEventSortKey;

WitnessEventSortKey witness_event_sort_key(const RenderedSourceUse & event)
{
  const ParsedLocation parsed = parse_line_col(event.location);
  const string::size_type split = event.location.rfind(':');
  const string::size_type path_split =
      split == string::npos ?
          string::npos : event.location.rfind(':', split - 1);
  const string path =
      path_split == string::npos ?
          event.location : event.location.substr(0, path_split);
  return std::make_tuple(
      path,
      parsed.line,
      parsed.column,
      witness_event_semantic_sort_rank(event),
      event.source_call_precedes_nested_callee ? 0 : 1,
      witness_event_source_sort_order(event),
      event.same_location_semantic_group_rank,
      binding_source_sort_rank(event),
      rendered_ownership_sort_key(event.ownership),
      !event.selected.empty() ? event.selected : event.template_name);
}

void sort_events(vector<RenderedSourceUse> & events)
{
  group_member_calls_with_source_owner_class_uses(events);
  group_materialized_class_uses_with_source_partial_class_uses(events);
  std::stable_sort(events.begin(),
                   events.end(),
                   [](const RenderedSourceUse & lhs, const RenderedSourceUse & rhs)
                   {
                     return witness_event_sort_key(lhs) <
                         witness_event_sort_key(rhs);
                   });
}

bool cv_type_atom_char(char ch)
{
  return std::isalnum(static_cast<unsigned char>(ch)) ||
         ch == '_' ||
         ch == ':';
}

size_t type_atom_begin_before(const string & text, size_t end)
{
  if(end == 0) {
    return string::npos;
  }
  if(text[end - 1] == '>') {
    int depth = 0;
    size_t begin = end;
    while(begin > 0) {
      --begin;
      if(text[begin] == '>') {
        ++depth;
      } else if(text[begin] == '<') {
        --depth;
        if(depth == 0) {
          while(begin > 0 && cv_type_atom_char(text[begin - 1])) {
            --begin;
          }
          return begin;
        }
      }
    }
    return string::npos;
  }
  if(!cv_type_atom_char(text[end - 1])) {
    return string::npos;
  }
  size_t begin = end;
  while(begin > 0 && cv_type_atom_char(text[begin - 1])) {
    --begin;
  }
  return begin;
}

string move_postfix_cv_before_type_atoms(string value, const string & cv)
{
  const string needle = " " + cv;
  size_t search = 0;
  while((search = value.find(needle, search)) != string::npos) {
    size_t atom_end = search;
    while(atom_end > 0 &&
          std::isspace(static_cast<unsigned char>(value[atom_end - 1]))) {
      --atom_end;
    }
    const size_t atom_begin = type_atom_begin_before(value, atom_end);
    if(atom_begin == string::npos || atom_begin >= atom_end) {
      search += needle.size();
      continue;
    }
    const size_t cv_end = search + needle.size();
    if(cv_end < value.size() && cv_type_atom_char(value[cv_end])) {
      search = cv_end;
      continue;
    }
    value = value.substr(0, atom_begin) + cv + " " +
            value.substr(atom_begin, atom_end - atom_begin) +
            value.substr(cv_end);
    search = atom_begin + cv.size() + 1 + (atom_end - atom_begin);
  }
  return value;
}

string normalize_const_order(const string & text)
{
  static map<string, string> cache;
  map<string, string>::const_iterator cached = cache.find(text);
  if(cached != cache.end()) {
    return cached->second;
  }
  static const std::regex const_volatile_suffix_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+const\\s+volatile\\b");
  static const std::regex volatile_const_suffix_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+volatile\\s+const\\b");
  static const std::regex parenthesized_indirection_volatile_const_regex(
      "(\\([^)]*[*^]\\s*)volatile\\s+const(\\))");
  static const std::regex const_before_indirection_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+const(\\s*[*&])");
  static const std::regex const_suffix_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+const\\b");
  static const std::regex volatile_before_indirection_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+volatile(\\s*[*&])");
  static const std::regex volatile_suffix_regex(
      "\\b(?!const\\b)(?!volatile\\b)([A-Za-z_][A-Za-z0-9_:]*)\\s+volatile\\b");
  string value = text;
  value = move_postfix_cv_before_type_atoms(value, "const");
  value = move_postfix_cv_before_type_atoms(value, "volatile");
  value = std::regex_replace(value, const_volatile_suffix_regex,
                             "const volatile $1");
  value = std::regex_replace(value, volatile_const_suffix_regex,
                             "const volatile $1");
  value = std::regex_replace(value,
                             parenthesized_indirection_volatile_const_regex,
                             "$1const volatile$2");
  value = std::regex_replace(value, const_before_indirection_regex,
                             "const $1$2");
  value = std::regex_replace(value, const_suffix_regex, "const $1");
  value = std::regex_replace(value, volatile_before_indirection_regex,
                             "volatile $1$2");
  value = std::regex_replace(value, volatile_suffix_regex, "volatile $1");
  cache[text] = value;
  return value;
}

string normalize_binding_arg_for_event(const string & arg);
string normalize_binding_arg_for_event(const string & arg,
                                       bool preserve_const_char_array,
                                       bool preserve_structured_type_spelling);
bool is_simple_identifier_text(const string & text);

string unqualified_template_name_text(const string & text)
{
  const string stripped = strip_top_level_template_suffix(trim_space(text));
  const string::size_type split = stripped.rfind("::");
  return split == string::npos ? stripped : stripped.substr(split + 2);
}

bool is_simple_identifier_text(const string & text)
{
  const string value = strip_simple_elaborated_prefix(text);
  if(value.empty()) {
    return false;
  }
  for(size_t i = 0; i < value.size(); ++i) {
    const char ch = value[i];
    if(i == 0) {
      if(!(std::isalpha(static_cast<unsigned char>(ch)) || ch == '_')) {
        return false;
      }
    } else if(!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')) {
      return false;
    }
  }
  return true;
}

void canonicalize_function_pointer_binding_args(vector<RenderedSourceUse> & events)
{
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::ClassUse &&
       events[i].kind != SourceUseKind::VariableUse) {
      continue;
    }
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      string arg = strip_simple_elaborated_prefix(trim_space(events[i].bindings[j].arg));
      if(arg.empty() || arg[0] == '&' || !is_simple_identifier_text(arg)) {
        continue;
      }
      if(events[i].bindings[j].function_pointer_parameter) {
        events[i].bindings[j].arg = "&" + arg;
      }
    }
  }
}

string normalize_binding_arg_for_event(const string & arg);
string normalize_binding_arg_for_event(const string & arg,
                                       bool preserve_const_char_array,
                                       bool preserve_structured_type_spelling);
string normalize_function_template_argument_spacing(const string & text);

void canonicalize_is_same_partial_bindings(vector<RenderedSourceUse> & events)
{
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::ClassUse ||
       events[i].selection != SourceSelectionKind::PartialSpecialization ||
       unqualified_template_name_text(events[i].template_name) != "is_same" ||
       events[i].bindings.size() != 2 ||
       events[i].specialization_bindings.size() != 1 ||
       events[i].specialization_bindings[0].arg.empty()) {
      continue;
    }
    const string canonical = events[i].specialization_bindings[0].arg;
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      if(events[i].bindings[j].source == "explicit") {
        events[i].bindings[j].arg = canonical;
      }
    }
  }
}

bool is_simple_qualified_identifier_text(const string & text)
{
  const string trimmed = trim_space(text);
  if(trimmed.find("::") == string::npos) {
    return false;
  }
  size_t start = 0;
  size_t part_count = 0;
  while(start <= trimmed.size()) {
    const size_t split = trimmed.find("::", start);
    const string part = split == string::npos ?
        trimmed.substr(start) : trimmed.substr(start, split - start);
    if(!is_simple_identifier_text(part)) {
      return false;
    }
    ++part_count;
    if(split == string::npos) {
      break;
    }
    start = split + 2;
  }
  return part_count >= 2;
}

string unqualified_identifier_tail(const string & text)
{
  const string trimmed = trim_space(text);
  const string::size_type split = trimmed.rfind("::");
  return split == string::npos ? trimmed : trimmed.substr(split + 2);
}

void collect_qualified_binding_alias(
    const RenderedSourceUse & event,
    const SourceBinding & binding,
    map<tuple<string, string, string>, string> & aliases,
    set<tuple<string, string, string> > & ambiguous)
{
  if(!is_simple_qualified_identifier_text(binding.arg)) {
    return;
  }
  const string tail = unqualified_identifier_tail(binding.arg);
  if(tail.empty()) {
    return;
  }
  const tuple<string, string, string> key =
      std::make_tuple(event.template_name, binding.param, tail);
  if(ambiguous.count(key) != 0) {
    return;
  }
  map<tuple<string, string, string>, string>::iterator found =
      aliases.find(key);
  if(found == aliases.end()) {
    aliases[key] = binding.arg;
    return;
  }
  if(found->second != binding.arg) {
    aliases.erase(found);
    ambiguous.insert(key);
  }
}

void apply_qualified_binding_alias(
    const RenderedSourceUse & event,
    SourceBinding & binding,
    const map<tuple<string, string, string>, string> & aliases)
{
  const string arg = trim_space(binding.arg);
  if(!is_simple_identifier_text(arg)) {
    return;
  }
  const tuple<string, string, string> key =
      std::make_tuple(event.template_name, binding.param, arg);
  map<tuple<string, string, string>, string>::const_iterator found =
      aliases.find(key);
  if(found != aliases.end()) {
    binding.arg = found->second;
  }
}

void canonicalize_qualified_binding_arguments(vector<RenderedSourceUse> & events)
{
  map<tuple<string, string, string>, string> aliases;
  set<tuple<string, string, string> > ambiguous;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind == SourceUseKind::AliasUse) {
      continue;
    }
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      collect_qualified_binding_alias(events[i],
                                      events[i].bindings[j],
                                      aliases,
                                      ambiguous);
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      collect_qualified_binding_alias(events[i],
                                      events[i].specialization_bindings[j],
                                      aliases,
                                      ambiguous);
    }
  }
  if(aliases.empty()) {
    return;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind == SourceUseKind::AliasUse) {
      continue;
    }
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      apply_qualified_binding_alias(events[i], events[i].bindings[j], aliases);
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      apply_qualified_binding_alias(events[i],
                                    events[i].specialization_bindings[j],
                                    aliases);
    }
  }
}

string header_from_kind(SourceUseKind kind)
{
  if(kind == SourceUseKind::ClassUse) {
    return "class-use";
  }
  if(kind == SourceUseKind::AliasUse) {
    return "alias-use";
  }
  if(kind == SourceUseKind::VariableUse) {
    return "variable-use";
  }
  return "function-call";
}

void normalize_event_names(vector<RenderedSourceUse> & events,
                           const vector<string> & names)
{
  for(size_t i = 0; i < events.size(); ++i) {
    events[i].template_name =
        witness_text::normalize_anonymous_namespace_segments(
            witness_text::strip_inline_namespace_segments(events[i].template_name,
                                                          names));
    events[i].selected =
        witness_text::normalize_anonymous_namespace_segments(
            witness_text::strip_inline_namespace_segments(events[i].selected,
                                                          names));
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      events[i].bindings[j].arg =
          witness_text::normalize_anonymous_namespace_segments(
              witness_text::strip_inline_namespace_segments(events[i].bindings[j].arg,
                                                            names));
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      events[i].specialization_bindings[j].arg =
          witness_text::normalize_anonymous_namespace_segments(
              witness_text::strip_inline_namespace_segments(
                  events[i].specialization_bindings[j].arg,
                  names));
    }
    for(size_t j = 0; j < events[i].drops.size(); ++j) {
      events[i].drops[j].candidate =
          witness_text::normalize_anonymous_namespace_segments(
              witness_text::strip_inline_namespace_segments(
                  events[i].drops[j].candidate,
                  names));
    }
  }
}

string normalize_binding_arg_for_event(const string & arg,
                                       bool preserve_const_char_array,
                                       bool preserve_structured_type_spelling)
{
  typedef tuple<string, bool, bool> CacheKey;
  static map<CacheKey, string> cache;
  const CacheKey key(arg,
                     preserve_const_char_array,
                     preserve_structured_type_spelling);
  map<CacheKey, string>::const_iterator cached = cache.find(key);
  if(cached != cache.end()) {
    return cached->second;
  }
  static const std::regex libcxx_namespace_regex("std::__1::");
  static const std::regex qualified_local_regex(
      "\\b([A-Za-z_][A-Za-z0-9_]*::)+([A-Za-z_][A-Za-z0-9_]*__local_\\d+)");
  static const std::regex local_regex("__local_\\d+");
  static const std::regex separated_angle_regex(">\\s+>");
  string value = arg;
  value = std::regex_replace(value, libcxx_namespace_regex, "std::");
  replace_all(value, "(*(&))", "(*&)");
  replace_all(value, "(*(&&))", "(*&&)");
  replace_all(value, "(^(&))", "(^&)");
  replace_all(value, "(^(&&))", "(^&&)");
  value = std::regex_replace(value, qualified_local_regex, "$2");
  value = std::regex_replace(value, local_regex, "");
  value = normalize_const_order(value);
  if(!preserve_structured_type_spelling) {
    value = normalize_source_event_type_spellings(value);
  }
  while(true) {
    const string collapsed =
        std::regex_replace(value, separated_angle_regex, ">>");
    if(collapsed == value) {
      break;
    }
    value = collapsed;
  }
  value = witness_text::normalize_source_spelling_text(value);
  const string trimmed = trim_space(value);
  if((trimmed.size() >= 17 && trimmed.compare(0, 16, "decltype(delete ") == 0) ||
     (trimmed.size() >= 19 && trimmed.compare(0, 18, "__decltype(delete ") == 0) ||
     (trimmed.size() >= 21 && trimmed.compare(0, 20, "__decltype__(delete ") == 0)) {
    cache[key] = "void";
    return "void";
  }
  const string normalized = normalize_function_template_argument_spacing(value);
  const string anonymous_normalized =
      witness_text::normalize_anonymous_namespace_segments(
          preserve_const_char_array ?
              normalized :
              normalize_string_literal_array_witness_text(normalized));
  cache[key] = anonymous_normalized;
  return anonymous_normalized;
}

bool function_template_name_before_angle(const string & text)
{
  size_t end = text.size();
  while(end > 0 && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }
  size_t begin = end;
  while(begin > 0) {
    const char ch = text[begin - 1];
    if(std::isalnum(static_cast<unsigned char>(ch)) ||
       ch == '_' ||
       ch == ':') {
      --begin;
      continue;
    }
    break;
  }
  const string name = text.substr(begin, end - begin);
  return name == "function" ||
         (name.size() > 10 &&
          name.compare(name.size() - 10, 10, "::function") == 0);
}

string normalize_function_template_argument_spacing(const string & text)
{
  string out;
  out.reserve(text.size());
  vector<char> angle_stack;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      angle_stack.push_back(function_template_name_before_angle(out));
      out.push_back(ch);
      continue;
    }
    if(ch == '>') {
      if(!angle_stack.empty()) {
        angle_stack.pop_back();
      }
      out.push_back(ch);
      continue;
    }
    if(ch == '(' &&
       !angle_stack.empty() &&
       angle_stack.back() &&
       !out.empty() &&
       !std::isspace(static_cast<unsigned char>(out[out.size() - 1]))) {
      out.push_back(' ');
    }
    out.push_back(ch);
  }
  return out;
}

string normalize_binding_arg_for_event(const string & arg)
{
  return normalize_binding_arg_for_event(arg, false, false);
}

string normalize_type_like_function_declarator_spacing(
    const string & text,
    bool function_type_argument)
{
  string out = trim_space(text);
  int angle_depth = 0;
  for(size_t i = 0; i < out.size(); ++i) {
    const char ch = out[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(ch == '(' &&
       angle_depth == 0 &&
       i > 0 &&
       !std::isspace(static_cast<unsigned char>(out[i - 1])) &&
       ((function_type_argument &&
         (std::isalnum(static_cast<unsigned char>(out[i - 1])) ||
          out[i - 1] == '_')) ||
        out[i - 1] == '>')) {
      out.insert(i, " ");
      break;
    }
  }
  return out;
}

string normalize_template_closing_angle_spacing(const string & text)
{
  string out;
  out.reserve(text.size());
  for(size_t i = 0; i < text.size(); ++i) {
    if(text[i] == '>') {
      while(!out.empty() &&
            std::isspace(static_cast<unsigned char>(out[out.size() - 1]))) {
        out.resize(out.size() - 1);
      }
    }
    out.push_back(text[i]);
  }
  return trim_space(out);
}

string normalize_entity_name_for_event(const string & entity)
{
  return normalize_template_closing_angle_spacing(
      normalize_binding_arg_for_event(entity));
}

string normalize_binding_arg_for_event(const SourceBinding & binding)
{
  const string normalized =
      normalize_binding_arg_for_event(binding.arg,
                                      binding.source == "explicit",
                                      binding.structured_type_spelling);
  const string normalized_only =
      normalized == "unsigned" ? "unsigned int" : normalized;
  if(binding.type_like) {
    return normalize_template_closing_angle_spacing(
        normalize_type_like_function_declarator_spacing(
            normalized_only,
            binding.function_type_argument));
  }
  return normalized_only;
}

string make_bound_template_text(const string & template_name,
                                const vector<SourceBinding> & bindings,
                                size_t count)
{
  string text = template_name;
  text += "<";
  for(size_t i = 0; i < count; ++i) {
    if(i != 0) {
      text += ", ";
    }
    text += bindings[i].arg;
  }
  text += ">";
  return text;
}

string source_line_location_key(const string & location)
{
  const ParsedLocation parsed = parse_line_col(location);
  if(parsed.line <= 0) {
    return string();
  }
  return location_source_path(location) + ":" + std::to_string(parsed.line);
}

string apply_text_aliases(const string & text,
                          const map<string, string> & aliases)
{
  string out = text;
  for(map<string, string>::const_iterator it = aliases.begin();
      it != aliases.end();
      ++it) {
    replace_all(out, it->first, it->second);
  }
  return out;
}

map<string, string> build_defaulted_class_aliases(
    const vector<RenderedSourceUse> & events,
    bool include_concrete_explicit_specializations = true)
{
  map<string, string> aliases;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::ClassUse ||
       events[i].bindings.empty()) {
      continue;
    }
    if(!include_concrete_explicit_specializations &&
       events[i].selection == SourceSelectionKind::ExplicitSpecialization) {
      const ParsedLocation use = parse_line_col(events[i].location);
      const ParsedLocation decl =
          parse_line_col(rendered_selected_decl_location(events[i]));
      const bool source_omitted_trailing_arguments =
          events[i].template_id_occurrence.present &&
          events[i].template_id_occurrence.arguments.size() <
              events[i].bindings.size();
      if((use.line <= 0 || decl.line <= 0 || decl.line <= use.line) &&
         !source_omitted_trailing_arguments) {
        continue;
      }
    }
    size_t keep_count = events[i].bindings.size();
    while(keep_count > 0 &&
          events[i].bindings[keep_count - 1].source == "defaulted") {
      --keep_count;
    }
    if(keep_count == events[i].bindings.size()) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    const string shortened =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 keep_count);
    aliases[full] = shortened;
  }
  return aliases;
}

map<string, string> build_predecl_all_defaulted_class_aliases(
    const vector<RenderedSourceUse> & events)
{
  map<string, string> aliases;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::ClassUse || events[i].bindings.empty()) {
      continue;
    }
    bool all_defaulted = true;
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      if(events[i].bindings[j].source != "defaulted") {
        all_defaulted = false;
        break;
      }
    }
    if(!all_defaulted) {
      continue;
    }
    const ParsedLocation raw =
        parse_line_col(events[i].location);
    const ParsedLocation decl =
        parse_line_col(rendered_selected_decl_location(events[i]));
    if(raw.line <= 0 || decl.line <= 0 || raw.line >= decl.line) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    const string shortened =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 0);
    aliases[full] = shortened;
  }
  return aliases;
}

bool source_has_empty_template_id_at_event_location(const RenderedSourceUse & event)
{
  const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
      event.template_id_occurrence;
  return occurrence.present &&
         occurrence.source_spelled &&
         occurrence.argument_list_spelled &&
         occurrence.empty_argument_list;
}

map<string, string> build_omitted_all_defaulted_class_aliases(
    const vector<RenderedSourceUse> & events)
{
  map<string, string> aliases;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::ClassUse || events[i].bindings.empty()) {
      continue;
    }
    bool all_defaulted = true;
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      if(events[i].bindings[j].source != "defaulted") {
        all_defaulted = false;
        break;
      }
    }
    if(!all_defaulted ||
       !source_has_empty_template_id_at_event_location(events[i])) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    const string shortened =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 0);
    aliases[full] = shortened;
  }
  return aliases;
}

map<string, string> build_function_pointer_class_aliases(
    const vector<RenderedSourceUse> & events)
{
  map<string, string> aliases;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::ClassUse || events[i].bindings.empty()) {
      continue;
    }
    vector<SourceBinding> unqualified_bindings = events[i].bindings;
    bool changed = false;
    for(size_t j = 0; j < unqualified_bindings.size(); ++j) {
      const string arg = trim_space(unqualified_bindings[j].arg);
      if(arg.size() <= 1 || arg[0] != '&') {
        continue;
      }
      const string without_amp = trim_space(arg.substr(1));
      if(!is_simple_identifier_text(without_amp)) {
        continue;
      }
      unqualified_bindings[j].arg = without_amp;
      changed = true;
    }
    if(!changed) {
      continue;
    }
    const string canonical =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    const string shorthand =
        make_bound_template_text(events[i].template_name,
                                 unqualified_bindings,
                                 unqualified_bindings.size());
    aliases[shorthand] = canonical;
  }
  return aliases;
}

void apply_binding_aliases(vector<RenderedSourceUse> & events,
                          const map<string, string> & aliases)
{
  if(aliases.empty()) {
    return;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      events[i].bindings[j].arg =
          apply_text_aliases(events[i].bindings[j].arg, aliases);
      for(size_t k = 0; k < events[i].bindings[j].pack_arguments.size(); ++k) {
        events[i].bindings[j].pack_arguments[k] =
            apply_text_aliases(events[i].bindings[j].pack_arguments[k],
                               aliases);
      }
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      events[i].specialization_bindings[j].arg =
          apply_text_aliases(events[i].specialization_bindings[j].arg, aliases);
      for(size_t k = 0;
          k < events[i].specialization_bindings[j].pack_arguments.size();
          ++k) {
        events[i].specialization_bindings[j].pack_arguments[k] =
            apply_text_aliases(
                events[i].specialization_bindings[j].pack_arguments[k],
                aliases);
      }
    }
    events[i].template_name =
        apply_text_aliases(events[i].template_name, aliases);
    if(!events[i].selected.empty()) {
      events[i].selected =
          apply_text_aliases(events[i].selected, aliases);
    }
    for(size_t j = 0; j < events[i].drops.size(); ++j) {
      events[i].drops[j].candidate =
          apply_text_aliases(events[i].drops[j].candidate, aliases);
    }
  }
}

void apply_event_name_aliases(vector<RenderedSourceUse> & events,
                              const map<string, string> & aliases)
{
  if(aliases.empty()) {
    return;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    events[i].template_name =
        apply_text_aliases(events[i].template_name, aliases);
    if(!events[i].selected.empty()) {
      events[i].selected =
          apply_text_aliases(events[i].selected, aliases);
    }
    for(size_t j = 0; j < events[i].drops.size(); ++j) {
      events[i].drops[j].candidate =
          apply_text_aliases(events[i].drops[j].candidate, aliases);
    }
  }
}

set<string> same_line_explicit_default_alias_uses(
    const vector<RenderedSourceUse> & events,
    const map<string, string> & aliases)
{
  set<string> out;
  if(aliases.empty()) {
    return out;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::ClassUse ||
       !events[i].template_id_occurrence.present ||
       !events[i].template_id_occurrence.source_spelled) {
      continue;
    }
    bool has_defaulted_binding = false;
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      if(events[i].bindings[j].source == "defaulted") {
        has_defaulted_binding = true;
        break;
      }
    }
    if(has_defaulted_binding) {
      continue;
    }
    const string line_key = source_line_location_key(events[i].location);
    if(line_key.empty()) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    if(aliases.find(full) != aliases.end()) {
      out.insert(line_key + "\x1f" + full);
    }
  }
  return out;
}

map<string, int> explicit_specialization_default_alias_pattern_lines(
    const vector<RenderedSourceUse> & events,
    const map<string, string> & aliases)
{
  map<string, int> out;
  if(aliases.empty()) {
    return out;
  }
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::ClassUse ||
       events[i].selection != SourceSelectionKind::ExplicitSpecialization) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    if(aliases.find(full) == aliases.end()) {
      continue;
    }
    const ParsedLocation use = parse_line_col(events[i].location);
    const ParsedLocation decl =
        parse_line_col(rendered_selected_decl_location(events[i]));
    if(use.line <= 0 || decl.line <= 0 || decl.line >= use.line) {
      continue;
    }
    map<string, int>::iterator found = out.find(full);
    if(found == out.end() || use.line < found->second) {
      out[full] = use.line;
    }
  }
  return out;
}

string apply_defaulted_aliases_to_binding_arg(
    const string & arg,
    const string & source,
    const string & line_key,
    int line_number,
    const map<string, string> & aliases,
    const set<string> & protected_explicit_uses,
    const map<string, int> & explicit_specialization_pattern_lines)
{
  string out = arg;
  for(map<string, string>::const_iterator it = aliases.begin();
      it != aliases.end();
      ++it) {
    map<string, int>::const_iterator pattern_line =
        explicit_specialization_pattern_lines.find(it->first);
    if(pattern_line != explicit_specialization_pattern_lines.end() &&
       line_number > pattern_line->second) {
      continue;
    }
    if(source == "explicit" &&
       !line_key.empty() &&
       protected_explicit_uses.count(line_key + "\x1f" + it->first) != 0) {
      continue;
    }
    replace_all(out, it->first, it->second);
  }
  return out;
}

void apply_defaulted_binding_aliases(vector<RenderedSourceUse> & events,
                                     const map<string, string> & aliases)
{
  if(aliases.empty()) {
    return;
  }
  const set<string> protected_explicit_uses =
      same_line_explicit_default_alias_uses(events, aliases);
  const map<string, int> explicit_specialization_pattern_lines =
      explicit_specialization_default_alias_pattern_lines(events, aliases);
  for(size_t i = 0; i < events.size(); ++i) {
    const string line_key = source_line_location_key(events[i].location);
    const int line_number = parse_line_col(events[i].location).line;
    for(size_t j = 0; j < events[i].bindings.size(); ++j) {
      events[i].bindings[j].arg =
          apply_defaulted_aliases_to_binding_arg(
              events[i].bindings[j].arg,
              events[i].bindings[j].source,
              line_key,
              line_number,
              aliases,
              protected_explicit_uses,
              explicit_specialization_pattern_lines);
      for(size_t k = 0; k < events[i].bindings[j].pack_arguments.size(); ++k) {
        events[i].bindings[j].pack_arguments[k] =
            apply_defaulted_aliases_to_binding_arg(
                events[i].bindings[j].pack_arguments[k],
                events[i].bindings[j].source,
                line_key,
                line_number,
                aliases,
                protected_explicit_uses,
                explicit_specialization_pattern_lines);
      }
    }
    for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
      events[i].specialization_bindings[j].arg =
          apply_defaulted_aliases_to_binding_arg(
              events[i].specialization_bindings[j].arg,
              events[i].specialization_bindings[j].source,
              line_key,
              line_number,
              aliases,
              protected_explicit_uses,
              explicit_specialization_pattern_lines);
      for(size_t k = 0;
          k < events[i].specialization_bindings[j].pack_arguments.size();
          ++k) {
        events[i].specialization_bindings[j].pack_arguments[k] =
            apply_defaulted_aliases_to_binding_arg(
                events[i].specialization_bindings[j].pack_arguments[k],
                events[i].specialization_bindings[j].source,
                line_key,
                line_number,
                aliases,
                protected_explicit_uses,
                explicit_specialization_pattern_lines);
      }
    }
  }
}

void normalize_event_bindings(vector<RenderedSourceUse> & events)
{
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::AliasUse) {
      for(size_t j = 0; j < events[i].bindings.size(); ++j) {
        events[i].bindings[j].arg =
            normalize_binding_arg_for_event(events[i].bindings[j]);
      }
      for(size_t j = 0; j < events[i].specialization_bindings.size(); ++j) {
        events[i].specialization_bindings[j].arg =
            normalize_binding_arg_for_event(
                events[i].specialization_bindings[j]);
      }
    }
    events[i].template_name =
        normalize_entity_name_for_event(events[i].template_name);
    if(!events[i].selected.empty()) {
      events[i].selected =
          normalize_entity_name_for_event(events[i].selected);
    }
    for(size_t j = 0; j < events[i].drops.size(); ++j) {
      events[i].drops[j].candidate =
          normalize_entity_name_for_event(events[i].drops[j].candidate);
    }
  }
  canonicalize_function_pointer_binding_args(events);
  canonicalize_is_same_partial_bindings(events);
  canonicalize_qualified_binding_arguments(events);
  const map<string, string> aliases = build_defaulted_class_aliases(events);
  apply_event_name_aliases(events, aliases);
  apply_defaulted_binding_aliases(events, aliases);
  apply_binding_aliases(events, build_function_pointer_class_aliases(events));
  apply_binding_aliases(events, build_predecl_all_defaulted_class_aliases(events));
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::ClassUse ||
       events[i].selection != SourceSelectionKind::Primary ||
       !events[i].specialization_bindings.empty() ||
       events[i].bindings.empty()) {
      continue;
    }
    const string full =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    map<string, string>::const_iterator found = aliases.find(full);
    if(found == aliases.end()) {
      continue;
    }
    size_t keep_count = events[i].bindings.size();
    while(keep_count > 0) {
      if(make_bound_template_text(events[i].template_name,
                                  events[i].bindings,
                                  keep_count) == found->second) {
        break;
      }
      --keep_count;
    }
    if(keep_count == 0 || keep_count == events[i].bindings.size()) {
      continue;
    }
    const size_t source_argument_count =
        events[i].template_id_occurrence.present &&
                events[i].template_id_occurrence.source_spelled ?
            events[i].template_id_occurrence.arguments.size() :
            0;
    for(size_t j = keep_count; j < events[i].bindings.size(); ++j) {
      if(j < source_argument_count &&
         events[i].bindings[j].source == "explicit") {
        continue;
      }
      events[i].bindings[j].source = "defaulted";
    }
  }

}

string source_location_compare_key(const string & location)
{
  const ParsedLocation parsed = parse_line_col(location);
  if(parsed.line <= 0 || parsed.column <= 0) {
    return normalize_witness_path(location);
  }
  return location_source_path(location) + ":" +
      std::to_string(parsed.line) + ":" + std::to_string(parsed.column);
}

string render_events_text(const vector<RenderedSourceUse> & events,
                          bool debug)
{
  vector<RenderedSourceUse> ordered = events;
  for(size_t i = 0; i < ordered.size(); ++i) {
    ordered[i].location = source_location_compare_key(ordered[i].location);
    for(size_t j = 0; j < ordered[i].drops.size(); ++j) {
      ordered[i].drops[j].location =
          source_location_compare_key(ordered[i].drops[j].location);
    }
  }
  std::ostringstream out;
  out << "translation-unit\n";
  for(size_t i = 0; i < ordered.size(); ++i) {
    const RenderedSourceUse & event = ordered[i];
    out << "  " << header_from_kind(event.kind) << " at " << event.location << "\n";
    if(event.kind == SourceUseKind::FunctionCall) {
      out << "    callee "
          << (!event.selected.empty() ? event.selected : event.template_name)
          << "\n";
    } else {
      out << "    template " << event.template_name << "\n";
    }
    if(event.selection != SourceSelectionKind::None) {
      out << "    selected " << witness_selection_text(event) << "\n";
    }
    const string selected_decl_location = source_location_compare_key(
        rendered_selected_decl_location(event));
    if(debug && !selected_decl_location.empty()) {
      out << "    decl " << selected_decl_location << "\n";
    }
    if(debug && event.candidates_built >= 0) {
      out << "    candidates_built " << event.candidates_built << "\n";
    }
    if(debug && event.candidates_viable >= 0) {
      out << "    candidates_viable " << event.candidates_viable << "\n";
    }
    for(size_t j = 0; j < event.bindings.size(); ++j) {
      out << "    bind " << (debug ? event.bindings[j].param :
                             ("#" + std::to_string(j + 1))) << " = "
          << event.bindings[j].arg << " source=" << event.bindings[j].source
          << "\n";
    }
    for(size_t j = 0; j < event.specialization_bindings.size(); ++j) {
      out << "    specialize "
          << (debug ? event.specialization_bindings[j].param :
              ("#" + std::to_string(j + 1))) << " = "
          << event.specialization_bindings[j].arg << " source="
          << event.specialization_bindings[j].source << "\n";
    }
    for(size_t j = 0; j < event.drops.size(); ++j) {
      out << "    drop " << event.drops[j].candidate;
      if(debug) {
        out << " at " << event.drops[j].location;
      }
      out << " reason=" << event.drops[j].reason << "\n";
    }
  }
  return out.str();
}

void collect_rendered_source_events(const template_api::TemplateWitnessSession & session,
                                    vector<RenderedSourceUse> & events)
{
  events.reserve(session.source_use_table.uses.size());
  for(size_t i = 0; i < session.source_use_table.uses.size(); ++i) {
    events.emplace_back(session.source_use_table.uses[i]);
  }
  normalize_event_names(events, session.inline_namespace_names);
  normalize_event_bindings(events);
  sort_events(events);
}

void record_explicit_source_owner_entity(set<string> & out,
                                         const string & owner_text)
{
  const string owner =
      template_api::template_witness_detail::normalize_template_log_entity(
          owner_text);
  if(owner.empty()) {
    return;
  }
  out.insert(owner);
  const string stripped = strip_top_level_template_suffix(owner);
  if(!stripped.empty()) {
    out.insert(stripped);
  }
}

}  // namespace

namespace template_api {

std::string render_template_source_witness_text(
    const TemplateWitnessSession & session,
    const std::string & source_path)
{
  vector<RenderedSourceUse> events;
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  collect_rendered_source_events(session, events);
  const string rendered = render_events_text(events, false);
  witness_provenance::finish_session(session, source_path);
  return rendered;
#else
  collect_rendered_source_events(session, events);
  return render_events_text(events, false);
#endif
}

std::string render_template_source_witness_debug_text(
    const TemplateWitnessSession & session,
    const std::string & source_path)
{
  vector<RenderedSourceUse> events;
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  collect_rendered_source_events(session, events);
  const string rendered = render_events_text(events, true);
  witness_provenance::finish_session(session, source_path);
  return rendered;
#else
  collect_rendered_source_events(session, events);
  return render_events_text(events, true);
#endif
}

std::map<std::string, std::string> template_source_defaulted_aliases(
    const TemplateWitnessSession & session)
{
  vector<RenderedSourceUse> events;
  collect_rendered_source_events(session, events);
  map<string, string> aliases = build_defaulted_class_aliases(events, false);
  const map<string, string> predecl_all_defaulted =
      build_predecl_all_defaulted_class_aliases(events);
  aliases.insert(predecl_all_defaulted.begin(), predecl_all_defaulted.end());
  const map<string, string> omitted_all_defaulted =
      build_omitted_all_defaulted_class_aliases(events);
  aliases.insert(omitted_all_defaulted.begin(), omitted_all_defaulted.end());
  const map<string, string> function_pointer_aliases =
      build_function_pointer_class_aliases(events);
  aliases.insert(function_pointer_aliases.begin(), function_pointer_aliases.end());
  return aliases;
}

std::set<std::string> template_source_owner_entities(
    const TemplateWitnessSession & session)
{
  vector<RenderedSourceUse> events;
  collect_rendered_source_events(session, events);
  set<string> out;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind == SourceUseKind::FunctionCall) {
      const string owner =
          !events[i].selected.empty() ? events[i].selected : events[i].template_name;
      if(!owner.empty()) {
        out.insert(
            template_api::template_witness_detail::normalize_template_log_entity(
                owner));
      }
      continue;
    }
    if(events[i].kind != SourceUseKind::ClassUse &&
       events[i].kind != SourceUseKind::AliasUse &&
       events[i].kind != SourceUseKind::VariableUse) {
      continue;
    }
    if(events[i].template_name.empty()) {
      continue;
    }
    if(events[i].bindings.empty()) {
      out.insert(template_api::template_witness_detail::normalize_template_log_entity(
          events[i].template_name));
      continue;
    }
    const string full_owner =
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size());
    out.insert(template_api::template_witness_detail::normalize_template_log_entity(
        full_owner));
    size_t visible_count = events[i].bindings.size();
    while(visible_count > 0 &&
          events[i].bindings[visible_count - 1].source == "defaulted") {
      --visible_count;
    }
    if(visible_count != events[i].bindings.size()) {
      out.insert(template_api::template_witness_detail::normalize_template_log_entity(
          make_bound_template_text(events[i].template_name,
                                   events[i].bindings,
                                   visible_count)));
    }
  }
  return out;
}

std::set<std::string> template_source_explicit_owner_entities(
    const TemplateWitnessSession & session)
{
  vector<RenderedSourceUse> events;
  collect_rendered_source_events(session, events);
  set<string> out;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].selection != SourceSelectionKind::ExplicitSpecialization) {
      continue;
    }
    if(events[i].kind == SourceUseKind::FunctionCall) {
      record_explicit_source_owner_entity(
          out,
          !events[i].selected.empty() ? events[i].selected : events[i].template_name);
      continue;
    }
    if(events[i].kind == SourceUseKind::ClassUse) {
      // A source class-use owns the class selection witness, not member
      // function materialization triggered by using that class.
      continue;
    }
    if(events[i].kind != SourceUseKind::AliasUse &&
       events[i].kind != SourceUseKind::VariableUse) {
      continue;
    }
    if(events[i].template_name.empty()) {
      continue;
    }
    if(events[i].bindings.empty()) {
      record_explicit_source_owner_entity(out, events[i].template_name);
      continue;
    }
    record_explicit_source_owner_entity(
        out,
        make_bound_template_text(events[i].template_name,
                                 events[i].bindings,
                                 events[i].bindings.size()));
  }
  return out;
}

std::set<std::string> template_source_argument_value_entities(
    const TemplateWitnessSession & session)
{
  vector<RenderedSourceUse> events;
  collect_rendered_source_events(session, events);
  set<string> out;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::ClassUse &&
       events[i].kind != SourceUseKind::AliasUse) {
      continue;
    }
    const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
        events[i].template_id_occurrence;
    if(!occurrence.present) {
      continue;
    }
    for(size_t j = 0; j < occurrence.arguments.size(); ++j) {
      const vector<string> & entities =
          occurrence.arguments[j].referenced_value_entities;
      for(size_t k = 0; k < entities.size(); ++k) {
        const string normalized =
            template_witness_detail::normalize_template_log_entity(entities[k]);
        if(!normalized.empty()) {
          out.insert(normalized);
        }
      }
    }
  }
  return out;
}

std::set<std::string> template_source_argument_value_decl_locations(
    const TemplateWitnessSession & session)
{
  vector<RenderedSourceUse> events;
  collect_rendered_source_events(session, events);
  set<string> out;
  for(size_t i = 0; i < events.size(); ++i) {
    if(events[i].kind != SourceUseKind::ClassUse &&
       events[i].kind != SourceUseKind::AliasUse) {
      continue;
    }
    const semantic_source_use::SourceTemplateIdOccurrence & occurrence =
        events[i].template_id_occurrence;
    if(!occurrence.present) {
      continue;
    }
    for(size_t j = 0; j < occurrence.arguments.size(); ++j) {
      const vector<string> & locations =
          occurrence.arguments[j].referenced_value_decl_locations;
      for(size_t k = 0; k < locations.size(); ++k) {
        const string normalized =
            normalize_template_witness_source_location(locations[k]);
        if(!normalized.empty()) {
          out.insert(normalized);
        }
      }
    }
  }
  return out;
}

}  // namespace template_api
